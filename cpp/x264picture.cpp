#include "x264picture.h"
#include "inputframe.h"
#include "inputframeplane.h"
#include <x264.h>

X264Picture::X264Picture()
{
	x264_picture_init(&m_picture);
}

void X264Picture::setInputFrame(InputFrame *inputFrame)
{
	m_picture.img.i_plane = inputFrame->planes();
	switch(inputFrame->colorSpace())
	{
		case VideoInput::Planar420:
			m_picture.img.i_csp = X264_CSP_I420;
			break;
	}
	m_picture.i_pts = inputFrame->presentationTimeStamp();

	for(int i=0; i<inputFrame->planes(); i++)
	{
		m_picture.img.i_stride[i] = inputFrame->plane(i)->width();
		m_picture.img.plane[i] = inputFrame->plane(i)->data();
	}
}

x264_picture_t *X264Picture::picture() 
{
	return &m_picture;
}
