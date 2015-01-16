#include <QtGui>

#include "frontends/qt4/mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	statusBar()->showMessage(tr("This is SuperGameHerm!  ^.^"));

	setWindowTitle("SuperGameHerm!");
}

MainWindow::~MainWindow()
{
}
