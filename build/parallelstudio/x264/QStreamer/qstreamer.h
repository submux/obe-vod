#ifndef QSTREAMER_H
#define QSTREAMER_H

#include <QtGui/QMainWindow>
#include "ui_qstreamer.h"

class QStreamer : public QMainWindow
{
	Q_OBJECT

public:
	QStreamer(QWidget *parent = 0, Qt::WFlags flags = 0);
	~QStreamer();

private:
	Ui::QStreamerClass ui;
};

#endif // QSTREAMER_H
