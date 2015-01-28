#ifndef __MAINWINDOW_H_
#define __MAINWINDOW_H_

#include <QMainWindow>

#include "frontends/qt/emuthread.h"
#include "frontends/qt/lcdwidget.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = NULL);
	~MainWindow();

private slots:
	void openRom();
	void quickOpenRom();
	void saveState();
	void closeRom();
	void quitSGH();

	void showDebugger();
	void romContinue();
	void romBreak();

	void openDocs();
	void fileBug();
	void showAbout();

	void frameRendered();

private:
	void initActions();
	void initMenus();
	void initWidgets();

	void toggleOpenROM(bool);

	QMenu *romMenu;
	QAction *openRomAction;
	QAction *quickOpenRomAction;
	QAction *saveStateAction;
	QAction *closeRomAction;
	QAction *quitAction;

	QMenu *debugMenu;
	QAction *showDebuggerAction;
	QAction *continueAction;
	QAction *breakAction;

	QMenu *helpMenu;
	QAction *openDocAction;
	QAction *fileBugAction;
	QAction *aboutAction;

	EmuThread *thread;
	LCDWidget *lcd;
};

#endif //__MAINWINDOW_H_
