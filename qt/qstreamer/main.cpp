#include "qstreamer.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QStreamer w;
	w.show();
	return a.exec();
}
