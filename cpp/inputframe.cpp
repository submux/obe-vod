#include "inputframe.h"
#include "inputframeplane.h"

InputFrame::InputFrame() :
	m_colorSpace(VideoInput::Planar420),
	m_presentationTimeStamp(0)
{
}

InputFrame::~InputFrame()
{
}

uint32_t InputFrame::planes() const
{
	return m_planes.count();
}

class InputFramePlane *InputFrame::plane(uint32_t plane)
{
	return m_planes[plane];
}

bool InputFrame::addPlane(uint32_t width, uint32_t height)
{
	InputFramePlane *newPlane = new InputFramePlane(width, height);
	if(!newPlane)
		return false;

	m_planes.append(newPlane);
	return true;
}

VideoInput::ColorSpace InputFrame::colorSpace() const
{
	return m_colorSpace;
}

void InputFrame::setColorSpace(VideoInput::ColorSpace colorSpace)
{
	m_colorSpace = colorSpace;
}

int64_t InputFrame::presentationTimeStamp() const
{
	return m_presentationTimeStamp;
}

void InputFrame::setPresentationTimeStamp(int64_t presentationTimeStamp)
{
	m_presentationTimeStamp = presentationTimeStamp;
}