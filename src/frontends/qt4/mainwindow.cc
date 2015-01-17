#include <QtGui>

#include "frontends/qt4/mainwindow.h"
#include "frontends/qt4/emuthread.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	initActions();
	initMenus();
	initWidgets();

	statusBar()->showMessage(tr("This is SuperGameHerm!  ^.^"));

	setWindowTitle("SuperGameHerm!");
}

MainWindow::~MainWindow()
{
}

/*!
 * @brief Initialise the Qt action objects.
 */
void MainWindow::initActions()
{
	openRomAction = new QAction(tr("Open ROM..."), this);
	openRomAction->setShortcut(QKeySequence::Open);
	openRomAction->setStatusTip(tr("Open a ROM file and save file."));
	connect(openRomAction, SIGNAL(triggered()), this, SLOT(openRom()));

	quickOpenRomAction = new QAction(tr("Quick Open ROM..."), this);
	quickOpenRomAction->setStatusTip(tr("Open a ROM file."));
	connect(quickOpenRomAction, SIGNAL(triggered()), this, SLOT(quickOpenRom()));

	saveStateAction = new QAction(tr("Save Battery"), this);
	saveStateAction->setEnabled(false);
	saveStateAction->setShortcut(QKeySequence::Save);
	saveStateAction->setStatusTip(tr("Force saving the battery to disk."));
	connect(saveStateAction, SIGNAL(triggered()), this, SLOT(saveState()));

	closeRomAction = new QAction(tr("Close ROM"), this);
	closeRomAction->setEnabled(false);
	closeRomAction->setShortcut(QKeySequence::Close);
	closeRomAction->setStatusTip(tr("Close the current ROM and end emulation."));
	connect(closeRomAction, SIGNAL(triggered()), this, SLOT(closeRom()));

	quitAction = new QAction(tr("Quit SuperGameHerm!"), this);
	quitAction->setShortcut(QKeySequence::Quit);
	quitAction->setStatusTip(tr("Close all ROMs and quit SuperGameHerm!"));
	connect(quitAction, SIGNAL(triggered()), this, SLOT(quitSGH()));

	showDebuggerAction = new QAction(tr("Show Debugger"), this);
	showDebuggerAction->setEnabled(false);
	showDebuggerAction->setStatusTip(tr("Show the debugger window."));
	connect(showDebuggerAction, SIGNAL(triggered()), this, SLOT(showDebugger()));

	continueAction = new QAction(tr("Continue"), this);
	continueAction->setEnabled(false);
	connect(continueAction, SIGNAL(triggered()), this, SLOT(romContinue()));

	breakAction = new QAction(tr("Break"), this);
	breakAction->setEnabled(false);
	connect(breakAction, SIGNAL(triggered()), this, SLOT(romBreak()));

	openDocAction = new QAction(tr("SuperGameHerm! Help"), this);
	openDocAction->setShortcut(QKeySequence::HelpContents);
	openDocAction->setStatusTip(tr("Open SuperGameHerm! documentation."));
	connect(openDocAction, SIGNAL(triggered()), this, SLOT(openDocs()));

	fileBugAction = new QAction(tr("File a bug..."), this);
	fileBugAction->setStatusTip(tr("Open the bug tracker."));
	connect(fileBugAction, SIGNAL(triggered()), this, SLOT(fileBug()));

	aboutAction = new QAction(tr("About SuperGameHerm!"), this);
	aboutAction->setStatusTip(tr("Display program and library information."));
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));
}

/*!
 * @brief Initialise the Qt menus.
 */
void MainWindow::initMenus()
{
	romMenu = menuBar()->addMenu(tr("ROM"));
	romMenu->addAction(openRomAction);
	romMenu->addAction(quickOpenRomAction);
	romMenu->addAction(saveStateAction);
	romMenu->addAction(closeRomAction);
	romMenu->addSeparator();
	romMenu->addAction(quitAction);

	debugMenu = menuBar()->addMenu(tr("Debug"));
	debugMenu->addAction(showDebuggerAction);
	debugMenu->addSeparator();
	debugMenu->addAction(continueAction);
	debugMenu->addAction(breakAction);

	helpMenu = menuBar()->addMenu(tr("Help"));
	helpMenu->addAction(openDocAction);
	helpMenu->addSeparator();
	helpMenu->addAction(fileBugAction);
	helpMenu->addSeparator();
	helpMenu->addAction(aboutAction);
}

/*!
 * @brief Initialise the Qt widgets.
 */
void MainWindow::initWidgets()
{

}

/*!
 * @brief Allow the user to select a ROM file to open.
 */
void MainWindow::openRom()
{
	// todo
}

/*!
 * @brief Only ask the user for a ROM file, not a save file or boot ROM.
 */
void MainWindow::quickOpenRom()
{
	QString filter = tr("All Game Boy ROMs (*.gb *.gbc);;DMG ROMs (*.gb);;CGB ROMs (*.gbc)");
	QString romFile = QFileDialog::getOpenFileName(this,
						       tr("Open ROM File"),
						       0,
						       filter,
						       0,
						       QFileDialog::HideNameFilterDetails);

	EmuThread *thread = new EmuThread(romFile, 0, 0, this);
	if(thread->initialise())
	{
		thread->start();
	}
	else
	{
		delete thread;
	}
}

/*!
 * @brief Forces a sync of battery RAM to disk.
 */
void MainWindow::saveState()
{
	// memmap_sync(state, state->mbc.cart_ram, &(state->mbc.cart_mm_data));
	// state->mbc.dirty = false;
}

/*!
 * @brief Closes the current ROM / emulation core.
 */
void MainWindow::closeRom()
{
	// todo
}

/*!
 * @brief Quit the whole app.
 */
void MainWindow::quitSGH()
{
	// XXX todo
	this->close();
}

/*!
 * @brief Show the debugger window.
 * @note Automatically breaks when shown.
 */
void MainWindow::showDebugger()
{

}

/*!
 * @brief Continue the ROM, if debugger has stopped/broken in.
 */
void MainWindow::romContinue()
{

}

/*!
 * @brief Break into debugger at current ROM state.
 */
void MainWindow::romBreak()
{

}

/*!
 * @brief Open the help contents (currently just a FAQ...)
 */
void MainWindow::openDocs()
{
	QDesktopServices::openUrl(QUrl("http://foxkit.us/sgherm/help"));
}

/*!
 * @brief Open the bug tracker.
 * @note We don't use a direct link to GitHub because we may change some day.
 */
void MainWindow::fileBug()
{
	QDesktopServices::openUrl(QUrl("http://foxkit.us/sgherm/bug"));
}

/*!
 * @brief Show program/library version and information.
 */
void MainWindow::showAbout()
{
	QMessageBox::about(this, tr("About SuperGameHerm!"),
			   tr("<p>SuperGameHerm!</p><b>Copyright &copy; 2015 Andrew Wilcox and contributors.</b>"
			      "<p>License: NCSA</p>"));
}
