#include <QEventLoop>
#include <QProcess>
#include "frontends/qt/emuthread.h"
#include "frontends/sdl2/frontend.h"	// we use SDL2 audio

#include "print.h"

bool qt4_video_init(emu_state *state)
{
	return true;
}

void qt4_video_finish(emu_state *state)
{
}

void qt4_blit_canvas(emu_state *state)
{
	EmuThread *thread = static_cast<EmuThread *>(state->front.data);
	thread->blitHandler();
}

frontend_video_t qt4_video = {
	qt4_video_init,
	qt4_video_finish,
	qt4_blit_canvas
};

EmuThread::EmuThread(QString romPath, QString savePath, QString bootROM,
		     QObject *parent) :
	QThread(parent),
	pathToROM(romPath), pathToSave(savePath), pathToBootROM(bootROM)
{
	to_stdout = stdout;
	to_stderr = stderr;
}

bool EmuThread::initialise()
{
	// XXX
	// I HATE THIS
	const char *boot_rom = pathToBootROM.toAscii();
	const char *rom = pathToROM.toAscii();
	const char *save = pathToSave.toAscii();

	if(strlen(boot_rom) == 0) boot_rom = NULL;
	if(strlen(rom) == 0) rom = NULL;
	if(strlen(save) == 0) save = NULL;

	state = init_emulator(boot_rom, rom, save);

	// end hate
	// XXX

	if(state == NULL)
	{
		return false;
	}

	state->front.data = this;

	// why NULL_LOOP?  because we use thread->run().
	select_frontend_all(state, SDL2_AUDIO, &qt4_video, NULL_LOOP);

	return true;
}

void EmuThread::run()
{
	QEventLoop loop;
	go = true;
	int i = 0;

	while(go)
	{
		// process events every 64 ticks.  why?  idk.
		// I literally pulled this number out of my butt.  it works.
		// I doubt input could be processed any quicker any way.
		if(++i == 64)
		{
			loop.processEvents();
			i = 0;
		}
		step_emulator(state);
	}
}

void EmuThread::blitHandler()
{
	emit frameRendered();
}

uchar *EmuThread::frameLocation()
{
	if(state == NULL)
	{
		return NULL;
	}

	return reinterpret_cast<uchar *>(&state->lcdc.out);
}

EmuThread::~EmuThread()
{
	go = false;

	if(state != NULL)
	{
		finish_frontend(state);
		finish_emulator(state);
	}
}
