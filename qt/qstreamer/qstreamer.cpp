#include "qstreamer.h"
#include "streamingsession.h"

#include <mediasource/y4minput.h>
#include <mediasource/dektecsource.h>

QStreamer::QStreamer(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	connect(ui.actionNewSession, SIGNAL(triggered()), this, SLOT(newSession()));
	connect(ui.actionDektec, SIGNAL(triggered()), this, SLOT(runDektec()));

	m_dektecSource = new MediaSource::DektecVideoInputSource(this);
	m_dektecSource->source().connectTo(1);
}

QStreamer::~QStreamer()
{

}

void QStreamer::newSession()
{
	/*MediaSource::Y4MInput *input = new MediaSource::Y4MInput;
	if(!input->open("y:\\testimages\\riverbed.y4m"))
	{
		delete input;
		return;
	}*/

	StreamingSession *session = new StreamingSession(this);
	session->setVideoInput(m_dektecSource);

	session->start();
}

void QStreamer::runDektec()
{
	m_dektecSource->start();
}

