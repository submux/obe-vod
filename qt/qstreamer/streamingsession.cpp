#include "streamingsession.h"
#include <codec/x264/x264encoder.h>
#include <codec/x264/x264picture.h>
#include <mediasource/inputframe.h>

StreamingSession::StreamingSession(QObject *parent) : 
	QThread(parent),
	m_videoInput(0)
{
}

StreamingSession::~StreamingSession()
{
}

void StreamingSession::setVideoInput(MediaSource::VideoInput *videoInput)
{
	m_videoInput = videoInput;
}

void StreamingSession::run()
{
	X264::X264Encoder encoder;

	encoder.parameters().setResolution(m_videoInput->width(), m_videoInput->height());
	encoder.parameters().setFrameRate(m_videoInput->frameRateNumerator(), m_videoInput->frameRateDenominator());
	encoder.parameters().setVariableFrameRateInput(m_videoInput->variableFrameRate());
	encoder.parameters().setTimeBase(m_videoInput->timeBaseNumerator(), m_videoInput->timeBaseDenominator());
	encoder.parameters().setInterlaced(m_videoInput->interlaced());
	encoder.parameters().setTopFieldFirst(m_videoInput->topFieldFirst());
	//encoder.parameters().setFrameCount(m_videoInput->frameCount());

	MediaSource::InputFrame *frame = m_videoInput->allocateFrame();
	if(!frame)
		return;	

	encoder.startEncoding();

	uint32_t frameIndex = 0;
	while(true)
	{
		if(!m_videoInput->readFrame(frame, frameIndex))
			return;

		//frame->setPresentationTimeStamp(frameIndex);

		X264::X264Picture picture;
		picture.setInputFrame(frame);

		X264::X264Picture encodedPicture;
		int nal;
		int encodedCount = encoder.encode(picture, encodedPicture);

	}

	delete frame;

	encoder.finishEncoding();
	return;
}
