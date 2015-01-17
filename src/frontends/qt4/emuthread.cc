#include <QEventLoop>
#include <QProcess>
#include "frontends/qt4/emuthread.h"

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

	// why NULL_LOOP?  because we use thread->run().
	select_frontend_all(state, NULL_AUDIO, &qt4_video, NULL_LOOP);

	return true;
}

void EmuThread::run()
{
	QEventLoop loop;
	while(go)
	{
		loop.processEvents();
		step_emulator(state);
	}
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
