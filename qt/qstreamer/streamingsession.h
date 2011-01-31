#pragma once

#include <QtCore/QThread>

#include "../mediasource/videoinput.h"

class StreamingSession : public QThread
{
public:
	StreamingSession(QObject *parent);

	~StreamingSession();

	void setVideoInput(MediaSource::VideoInput *videoInput);

protected:
	void run();

private:
	MediaSource::VideoInput *m_videoInput;
	class X264Encoder *m_encoder;
};
