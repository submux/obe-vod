#ifndef QSTREAMER_H
#define QSTREAMER_H

#include <QtGui/QMainWindow>
#include "ui_qstreamer.h"

namespace MediaSource
{
	class DektecVideoInputSource;
}

class QStreamer : public QMainWindow
{
	Q_OBJECT

public:
	QStreamer(QWidget *parent = 0, Qt::WFlags flags = 0);
	~QStreamer();

public slots:
	void newSession();

	void runDektec();

private:
	Ui::QStreamerClass ui;

	MediaSource::DektecVideoInputSource *m_dektecSource;
};

#endif // QSTREAMER_H
