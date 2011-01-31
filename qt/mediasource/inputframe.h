#pragma once

#include <QtCore/QList>

#include <stdint.h>

#include "videoinput.h"

namespace MediaSource
{
	class InputFrame
	{
	public:
		InputFrame();

		~InputFrame();

		uint32_t planes() const;

		class InputFramePlane *plane(uint32_t plane);

		bool addPlane(uint32_t width, uint32_t height);

		VideoInput::ColorSpace colorSpace() const;

		void setColorSpace(VideoInput::ColorSpace colorSpace);

		int64_t presentationTimeStamp() const;

		void setPresentationTimeStamp(int64_t presentationTimeStamp);

		void take(InputFrame *source);

	private:
		QList<class InputFramePlane *>m_planes;
		VideoInput::ColorSpace m_colorSpace;
		int64_t m_presentationTimeStamp;
	};
}