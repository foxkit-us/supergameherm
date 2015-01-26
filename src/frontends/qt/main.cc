#include <QApplication>
#include "frontends/qt4/mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	MainWindow win;

	win.show();

	return app.exec();
}
