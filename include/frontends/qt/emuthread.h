#ifndef __EMUTHREAD_H__
#define __EMUTHREAD_H__

#include <QThread>
#include <QImage>

extern "C"
{
#	include "sgherm.h"
}

class EmuThread : public QThread
{
	Q_OBJECT

public:
	EmuThread(QString romPath, QString savePath = 0, QString bootROM = 0,
		  QObject *parent = 0);
	~EmuThread();

	uchar *frameLocation();

	bool initialise();

protected:
	void run();

signals:
	void frameRendered();

public slots:

private:
	void blitHandler();

	QString pathToROM;
	QString pathToSave;
	QString pathToBootROM;

	emu_state *state;
	bool go;

	friend void qt4_blit_canvas(emu_state *state);
};

#endif // !__EMUTHREAD_H__
