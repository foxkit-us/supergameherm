#ifndef __EMUTHREAD_H__
#define __EMUTHREAD_H__

#include <QThread>
extern "C" {
#include "sgherm.h"
}

class EmuThread : public QThread
{
	Q_OBJECT

public:
	EmuThread(QString romPath, QString savePath = 0, QString bootROM = 0,
		  QObject *parent = 0);
	~EmuThread();

	bool initialise();

protected:
	void run();

signals:

public slots:

private:
	QString pathToROM;
	QString pathToSave;
	QString pathToBootROM;

	emu_state *state;
	bool go = true;
};

#endif // !__EMUTHREAD_H__
