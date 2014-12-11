#include <windows.h>
#include "sgherm.h"

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

void w32_blit_canvas(emu_state *state)
{
	video_state *s = (video_state *)state->front.video.data;
	HDC hdc = GetDC(s->hWnd);

	SetBitmapBits(s->bm, 92960, (LPVOID)state->lcdc.out);

	BitBlt(hdc, 0, 0, 160, 144, s->mem, 0, 0, SRCCOPY);

	ReleaseDC(s->hWnd, hdc);
}

bool w32_init_input(emu_state *state)
{
	// nothing to init
	return true;
}

void w32_finish_input(emu_state *state)
{
	// nothing to deinit
}

void TranslateKeyToGameBoy(emu_state *state, WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
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
	case 'a':
	case 'Z':
	case 'z':
		state->front.input.data = (void *)INPUT_A;
		return;
	case 'S':
	case 's':
	case 'X':
	case 'x':
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

	if((g_state = init_emulator(rom_path, FRONT_WIN32, FRONT_NULL,
		FRONT_WIN32, FRONT_WIN32)) == NULL)
	{
		return -1;
	}

	free(rom_path);

	return main_common(g_state);
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
