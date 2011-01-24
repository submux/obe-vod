#include "x264encoder.h"
#include "x264picture.h"
#include "inputframe.h"

#include <stdio.h>

X264Encoder::X264Encoder() : m_encoder(0)
{
	
}

X264Encoder::~X264Encoder()
{
	finishEncoding();
}

X264Parameters &X264Encoder::parameters()
{
	return m_parameters;
}

bool X264Encoder::startEncoding()
{
	if(encoding())
		return false;

	m_encoder = x264_encoder_open(m_parameters.internalParameters());

	if(!m_encoder)
	{
		//x264_cli_log( "x264", X264_LOG_ERROR, "x264_encoder_open failed\n" );
		return false;
	}

	return true;
}

bool X264Encoder::finishEncoding()
{
	if(encoding())
		return true;

	

	return true;
}

bool X264Encoder::encoding() const
{
	return m_encoder ? true : false;
}

int X264Encoder::encode(X264Picture &input, X264Picture &output)
{
	x264_picture_t pic_out;
	int i_nal;
	int i_frame_size = 0;
	x264_nal_t *nal;

	int result =  x264_encoder_encode(m_encoder, &nal, &i_nal, input.picture(), output.picture());

	printf("Got NAL %d bytes\n", result);

	return result;
}
