#include "ThreeDViewer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QLocale::setDefault(QLocale::c());

	QCoreApplication::setOrganizationName("MPM");
	QCoreApplication::setApplicationName("ThreeDViewer");

	QApplication a(argc, argv);
	ThreeDViewer w;
	w.show();
	return a.exec();
}