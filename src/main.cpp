#include "3DViewer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QLocale::setDefault(QLocale::c());

	QCoreApplication::setOrganizationName("MPM");
	QCoreApplication::setApplicationName("3DViewer");

	QApplication a(argc, argv);
	3DViewer w;
	w.show();
	return a.exec();
}