#pragma once

#include <stdint.h>
#include <x264.h>

class X264Picture
{
public:
	X264Picture();

	void setInputFrame(class InputFrame *inputFrame);

protected:

	friend class X264Encoder;

	x264_picture_t *picture();

private:
	x264_picture_t m_picture;
};
