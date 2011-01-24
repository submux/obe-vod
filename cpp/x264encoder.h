#pragma once

#include <stdint.h>
#include <x264.h>

#include "x264parameters.h"

class X264Encoder
{
public:
	X264Encoder();

	~X264Encoder();

	X264Parameters &parameters();

	bool startEncoding();

	bool finishEncoding();

	bool encoding() const;

	int encode(class X264Picture &input, class X264Picture &output);

private:
	X264Parameters m_parameters;
	x264_t *m_encoder;
};
