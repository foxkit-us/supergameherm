#include <windows.h>
#include "sgherm.h"

LRESULT CALLBACK VViewProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HermProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

emu_state *g_state;

#include "print.h"	// debug
#include "frontend.h"	// frontend

extern bool do_exit;

typedef struct video_state
{
	/*! the window handle */
	HWND hWnd;
	/*! the memory DC for the window handle */
	HDC mem;
	/*! the bitmap blitted to mem every vblank */
	HBITMAP bm;
	/*! is vram viewer showing? */
	bool vramViewerIsActive;
	/*! vram viewer window maybe */
	HWND vramWindow;
	/*! vram viewer bitmap */
	HBITMAP vramBM;
} video_state;

bool w32_init_video(emu_state *state)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	WNDCLASSEX wndcl;
	HDC hdc;
	video_state *s;

	s = (video_state *)state->front.video.data = calloc(sizeof(video_state), 1);

	ZeroMemory(&wndcl, sizeof(wndcl));
	wndcl.cbSize = sizeof(wndcl);
	wndcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndcl.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wndcl.hInstance = hInstance;
	wndcl.lpfnWndProc = HermProc;
	wndcl.lpszClassName = "HermWindow";
	wndcl.style = CS_HREDRAW | CS_VREDRAW;

	if(!RegisterClassEx(&wndcl))
	{
		error(state, "registering window class: %04X\n", GetLastError());
		return false;
	}

	ZeroMemory(&wndcl, sizeof(wndcl));
	wndcl.cbSize = sizeof(wndcl);
	wndcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndcl.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wndcl.hInstance = hInstance;
	wndcl.lpfnWndProc = VViewProc;
	wndcl.lpszClassName = "VRAMView";
	wndcl.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wndcl))
	{
		error(state, "registering window class: %04X\n", GetLastError());
		return false;
	}


	s->hWnd = CreateWindowEx(WS_EX_CLIENTEDGE, "HermWindow", "Super Game Herm!",
		WS_BORDER | WS_CAPTION | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 160, 144, NULL, NULL, hInstance,
		NULL);
	if(s->hWnd == NULL)
	{
		error(state, "creating window: %04X\n", GetLastError());
		return false;
	}

	ShowWindow(s->hWnd, SW_NORMAL);
	UpdateWindow(s->hWnd);

	hdc = GetDC(s->hWnd);

	s->mem = CreateCompatibleDC(hdc);
	s->bm = CreateBitmap(160, 144, 1, 32, NULL);
	if(s->bm == NULL)
	{
		error(state, "creating bitmap: 0x%08X\n", GetLastError());
		return false;
	}
	SelectObject(s->mem, s->bm);

	s->vramBM = CreateBitmap(256, 256, 1, 32, NULL);

	ReleaseDC(s->hWnd, hdc);

	return true;
}

void w32_finish_video(emu_state *state)
{
	video_state *s = (video_state *)state->front.video.data;

	DeleteObject(s->bm);
	DeleteDC(s->mem);

	free(s);
	state->front.video.data = NULL;
}

uint32_t *GetPixelsForTiles(emu_state *state)
{
	uint32_t *fb = calloc(256 * 256, 4);

	uint16_t start = (state->lcdc.lcd_control.params.bg_char_sel) ? 0x0 : 0x800;
	uint32_t val[4] = { 0x00FFFFFF, 0x00AAAAAA, 0x00777777, 0x00000000 };
	uint8_t curr_tile = 0;
	uint8_t iter = 0, line = 0, row = 0, skip = 0;

	for (; row < 0xF; row++)
	{
		for (line = 0; line < 8; line++, skip = 0, curr_tile -= 8)
		{
			for (; iter < 0xF; curr_tile++, iter++)
			{
				uint8_t pixel_temp;
				uint16_t *mem;
				mem = (uint16_t *)(state->lcdc.vram[0x0] + start + (curr_tile * 16) + (line * 2));

				/*pixels = interleave(*mem);
				state->lcdc.out[skip][state->lcdc.ly] = val[(pixels & 0x3)];
				state->lcdc.out[skip + 1][state->lcdc.ly] = val[(pixels & 0x300 >> 8)];
				state->lcdc.out[skip + 2][state->lcdc.ly] = val[(pixels & 0x30000 >> 16)];
				state->lcdc.out[skip + 3][state->lcdc.ly] = val[(pixels & 0x3000000 >> 24)];

				pixels = interleave(*++mem);
				state->lcdc.out[skip + 4][state->lcdc.ly] = val[(pixels & 0x3)];
				state->lcdc.out[skip + 5][state->lcdc.ly] = val[(pixels & 0x300 >> 8)];
				state->lcdc.out[skip + 6][state->lcdc.ly] = val[(pixels & 0x30000 >> 16)];
				state->lcdc.out[skip + 7][state->lcdc.ly] = val[(pixels & 0x3000000 >> 24)];*/

				pixel_temp = ((*mem & 0x01) << 1) | (*mem & 0x100 >> 8);
				fb[row * line * 256 + skip++] = val[pixel_temp];
				pixel_temp = ((*mem & 0x02)) | ((*mem & 0x200) >> 9);
				fb[row * line * 256 + skip++] = val[pixel_temp];
				pixel_temp = ((*mem & 0x04) >> 1) | ((*mem & 0x400) >> 10);
				fb[row * line * 256 + skip++] = val[pixel_temp];
				pixel_temp = ((*mem & 0x08) >> 2) | ((*mem & 0x800) >> 11);
				fb[row * line * 256 + skip++] = val[pixel_temp];
				pixel_temp = ((*mem & 0x10) >> 3) | ((*mem & 0x1000) >> 12);
				fb[row * line * 256 + skip++] = val[pixel_temp];
				pixel_temp = ((*mem & 0x20) >> 4) | ((*mem & 0x2000) >> 13);
				fb[row * line * 256 + skip++] = val[pixel_temp];
				pixel_temp = ((*mem & 0x40) >> 5) | ((*mem & 0x4000) >> 14);
				fb[row * line * 256 + skip++] = val[pixel_temp];
				pixel_temp = ((*mem & 0x80) >> 6) | ((*mem & 0x8000) >> 15);
				fb[row * line * 256 + skip++] = val[pixel_temp];
			}
		}
	}
	return fb;
}

void w32_blit_canvas(emu_state *state)
{
	video_state *s = (video_state *)state->front.video.data;
	HDC hdc = GetDC(s->hWnd);

	SetBitmapBits(s->bm, 92960, (LPVOID)state->lcdc.out);

	BitBlt(hdc, 0, 0, 160, 144, s->mem, 0, 0, SRCCOPY);

	ReleaseDC(s->hWnd, hdc);

	if(s->vramViewerIsActive)
	{
		HDC hdc = GetDC(s->vramWindow);
		HDC mem = CreateCompatibleDC(hdc);
		uint32_t *stuff = GetPixelsForTiles(state);
		SelectObject(mem, s->vramBM);
		SetBitmapBits(s->vramBM, 262144, stuff);
		free(stuff);
		BitBlt(hdc, 0, 0, 256, 256, mem, 0, 0, SRCCOPY);
		ReleaseDC(s->vramWindow, hdc);
	}
}

bool w32_init_input(emu_state *state UNUSED)
{
	// nothing to init
	return true;
}

void w32_finish_input(emu_state *state UNUSED)
{
	// nothing to deinit
}

void ShowVRAMViewer(emu_state *state)
{
	video_state *s = (video_state *)state->front.video.data;
	s->vramWindow = CreateWindowEx(0, "VRAMView", "VRAM Viewer",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 256, 256,
		s->hWnd, NULL, GetModuleHandle(NULL), NULL);

	ShowWindow(s->vramWindow, SW_NORMAL);
	UpdateWindow(s->vramWindow);
}

void TranslateKeyToGameBoy(emu_state *state, WPARAM wParam, LPARAM lParam UNUSED)
{
	switch(wParam)
	{
	case VK_F9:
	{
		video_state *s = (video_state *)state->front.video.data;
		ShowVRAMViewer(state);
		s->vramViewerIsActive = true;
		return;
	}
	case VK_ESCAPE:
		PostQuitMessage(0);
		return;
	case VK_DOWN:
		state->front.input.data = (void *)INPUT_DOWN;
		return;
	case VK_UP:
		state->front.input.data = (void *)INPUT_UP;
		return;
	case VK_LEFT:
		state->front.input.data = (void *)INPUT_LEFT;
		return;
	case VK_RIGHT:
		state->front.input.data = (void *)INPUT_RIGHT;
		return;
	case 'A':
	case 'Z':
		state->front.input.data = (void *)INPUT_A;
		return;
	case 'S':
	case 'X':
		state->front.input.data = (void *)INPUT_B;
		return;
	case VK_RETURN:
		state->front.input.data = (void *)INPUT_START;
		return;
	case VK_BACK:
		state->front.input.data = (void *)INPUT_SELECT;
		return;
	}
}

void w32_get_key(emu_state *state, frontend_input_return *ret)
{
	ret->key = (input_key)(state->front.input.data);
	ret->press = true;
}

void StepEmulator(emu_state *state)
{
	step_emulator(state);
}

int w32_event_loop(emu_state *state)
{
	video_state *s = (video_state *)state->front.video.data;
	MSG msg;

	while(!do_exit)
	{
		StepEmulator(g_state);
		if(PeekMessage(&msg, s->hWnd, 0, 0, PM_REMOVE) > 0x0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}

const frontend_video w32_frontend_video = {
	&w32_init_video,
	&w32_finish_video,
	&w32_blit_canvas,
	NULL
};

const frontend_input w32_frontend_input = {
	&w32_init_input,
	&w32_finish_input,
	&w32_get_key,
	NULL
};

char *AskUserForROMPath(void)
{
	char szROMName[MAX_PATH];

	OPENFILENAME ofn;
	ZeroMemory(&szROMName, sizeof(szROMName));
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
	//ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szROMName;
	ofn.lpstrFilter = "All Game Boy ROMs\0*.gb;*.gbc\0Original Game Boy (DMG) ROMs\0*.gb\0";
	ofn.lpstrTitle = "Open Game!";
	ofn.nMaxFile = sizeof(szROMName);

	if(!GetOpenFileName(&ofn))
	{
		return NULL;
	}
	else
	{
		return strdup(szROMName);
	}
}

int WINAPI WinMain(HINSTANCE hInstance UNUSED, HINSTANCE hPrevInstance UNUSED, char *szCmdLine, int iCmdShow UNUSED)
{
	char *rom_path;

	AllocConsole();
	to_stdout = freopen("CONOUT$", "w", stdout);
	to_stderr = freopen("CONOUT$", "w", stderr);

	if(szCmdLine == NULL || strlen(szCmdLine) == 0)
	{
		rom_path = AskUserForROMPath();
	} else {
		rom_path = strdup(szCmdLine);
	}

	if((g_state = init_emulator(rom_path)) == NULL)
	{
		return -1;
	}

	free(rom_path);

	if(!select_frontend_all(state, FRONT_W32))
	{
		return -1;
	}

	if(!EVENT_LOOP(state))
	{
		fatal(state, "Emulator exited abnormally");
		return -1;
	}

	finish_emulator(state);

	return 0;
}


LRESULT CALLBACK VViewProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg)
	{
	/*case WM_PAINT:
	{
		if(lParam == NULL) return 0;

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		HDC mem = CreateCompatibleDC(hdc);
		uint32_t *pixels = GetPixelsForTiles((emu_state *)lParam);
		HBITMAP bm = CreateBitmap(256, 256, 1, 32, pixels);
		SelectObject(mem, bm);
		BitBlt(hdc, 0, 0, 256, 256, mem, 256, 256, SRCCOPY);
		DeleteObject(bm);
		free(pixels);
		DeleteDC(mem);
		EndPaint(hWnd, &ps);
		return 0;
	}*/
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;
		mmi->ptMaxSize.x = 256;
		mmi->ptMaxSize.y = 256;
		return 0;
	}
	case WM_CLOSE:
	{
		video_state *s = (video_state *)g_state->front.video.data;
		s->vramViewerIsActive = false;
		DestroyWindow(hWnd);
		return 0;
	}
	default:
		return DefWindowProc(hWnd, iMsg, wParam, lParam);
	}
}

LRESULT CALLBACK HermProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg)
	{
	case WM_KEYDOWN:
		TranslateKeyToGameBoy(g_state, wParam, lParam);
		return 0;
	case WM_KEYUP:
		g_state->front.input.data = NULL;
		return 0;
	case WM_CLOSE:
		do_exit = true;
		return 0;
	default:
		return DefWindowProc(hWnd, iMsg, wParam, lParam);
	}
}
