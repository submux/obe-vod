#include "y4minput.h"
#include "inputframe.h"
#include "inputframeplane.h"

#define Y4M_MAGIC "YUV4MPEG2"
#define MAX_YUV4_HEADER 80
#define Y4M_FRAME_MAGIC "FRAME"
#define MAX_FRAME_HEADER 80

Y4MInput::Y4MInput() : 
	VideoInput()		
{
	m_frameHeaderLength = strlen(Y4M_FRAME_MAGIC) + 1;
}

Y4MInput::~Y4MInput()
{
	close();
}

bool Y4MInput::open(const TString &fileName)
{
	if(!m_file.open(fileName, TIoDevice::ReadOnly))
		return false;

	TString header = m_file.readLine(MAX_YUV4_HEADER);
	if(!header.startsWith(Y4M_MAGIC))
	{
		m_file.close();
		return false;
	}

	m_sequenceHeaderLength = m_file.position();

	TStringList args = header.split(' ', false);

	int i=0;
	while(i<args.count())
	{
		if(args[i].startsWith("W"))
			setWidth(args[i].mid(1).toInteger());
		else if(args[i].startsWith("H"))
			setHeight(args[i].mid(1).toInteger());
		else if(args[i].startsWith("F"))
		{
			TStringList frameRateComponents = args[i].mid(1).split(':');
			if(frameRateComponents.count() != 2)
				return false;

			setFrameRate(frameRateComponents[0].toInteger(), frameRateComponents[1].toInteger());
		}
		else if(args[i].startsWith("I"))
		{
			switch(args[i][1].toLatin1())
			{
				case 't':
					setTopFieldFirst();
					break;
				case 'b':
					setBottomFieldFirst();
					break;
				case 'm':
					setMixedInterlacing();
					break;
				case 'p':
				default:
					setProgressive();
			}
		}
		else if(args[i].startsWith("A"))
		{
			TStringList aspectRatioComponents = args[i].mid(1).split(':');
			if(aspectRatioComponents.count() != 2)
				return false;

			setAspectRatio(aspectRatioComponents[0].toInteger(), aspectRatioComponents[1].toInteger());
		}
		i++;
	}

	m_frameSize = m_frameHeaderLength;
	for(int i=0; i<planes(); i++)
		m_frameSize += planeSize(i);

	uint64_t start = m_file.position();
	uint64_t end = m_file.size();

	m_frameCount = (end - start) / m_frameSize;

	m_nextFrame = 0;

	return true;
}

void Y4MInput::close()
{
	if(!isOpen())
		return;

	m_file.close();
}

bool Y4MInput::isOpen() const
{
	return m_file.isOpen();
}

bool Y4MInput::seekFrame(uint64_t index)
{
	if(index == m_nextFrame)
		return true;

	uint64_t framePosition = m_sequenceHeaderLength + (index * m_frameSize);

	m_file.seek(framePosition);
	if(m_file.position() != framePosition)
		return false;
	
	return true;
}

bool Y4MInput::readFrame(InputFrame *result, uint64_t index)
{
	if(!seekFrame(index))
		return false;

	TString header = m_file.readLine(16);
	if(!header.startsWith("FRAME"))
		return false;

	for(int i=0; i<planes(); i++)
	{
		uint32_t toRead = planeSize(i);
		if(m_file.read(result->plane(i)->data(), toRead) != toRead)
			return false;
	}

	m_nextFrame++;

	return true;
}

InputFrame *Y4MInput::allocateFrame()
{
	InputFrame *newFrame = new InputFrame;
	newFrame->setColorSpace(colorSpace());
	for(int i=0; i<planes(); i++)
	{
		if(!newFrame->addPlane(planeWidth(i), planeHeight(i)))
		{
			delete newFrame;
			return 0;
		}
	}
	return newFrame;
}

uint64_t Y4MInput::frameCount() const
{
	return m_frameCount;
}

