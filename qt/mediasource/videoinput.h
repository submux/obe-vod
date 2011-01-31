#pragma once

#include <stdint.h>
#include "mediainput.h"

namespace MediaSource
{
	class VideoInput : public MediaInput
	{
	public:
		enum InterlaceMode
		{
			TopFieldFirst,
			BottomFieldFirst,
			MixedInterlacing,
			Progressive
		};

		enum ColorSpace
		{
			Planar420
		};

		VideoInput(QObject *parent=0) :
			MediaInput(parent),
			m_width(0),
			m_height(0),
			m_frameRateNumerator(0),
			m_frameRateDenominator(0),
			m_variableFrameRate(false),
			m_timeBaseNumerator(0),
			m_timeBaseDenominator(0),
			m_interlaceMode(Progressive),
			m_aspectRatioWidth(1),
			m_aspectRatioHeight(1),
			m_colorSpace(Planar420)
		{
		}

		void setWidth(uint32_t width)
		{
			m_width = width;
		}

		uint32_t width() const
		{
			return m_width;
		}

		void setHeight(uint32_t height)
		{
			m_height = height;
		}

		uint32_t height() const
		{
			return m_height;
		}

		void setFrameRate(uint32_t numerator, uint32_t denominator)
		{
			m_frameRateNumerator = numerator;
			m_frameRateDenominator = denominator;
		}

		void setFrameRateNumerator(uint32_t frameRateNumerator)
		{
			m_frameRateNumerator = frameRateNumerator;
		}

		uint32_t frameRateNumerator() const
		{
			return m_frameRateNumerator;
		}

		void setFrameRateDenominator(uint32_t frameRateDenominator)
		{
			m_frameRateDenominator = frameRateDenominator;
		}

		uint32_t frameRateDenominator() const
		{
			return m_frameRateDenominator;
		}

		void setVariableFrameRate(bool variableFrameRate)
		{
			m_variableFrameRate = variableFrameRate;
		}

		bool variableFrameRate() const
		{
			return m_variableFrameRate;
		}

		void setTimeBase(uint32_t numerator, uint32_t denominator)
		{
			m_timeBaseNumerator = numerator;
			m_timeBaseDenominator = denominator;
		}

		void setTimeBaseNumerator(uint32_t timeBaseNumerator)
		{
			m_timeBaseNumerator = timeBaseNumerator;
		}

		uint32_t timeBaseNumerator() const
		{
			if(!m_variableFrameRate)
				return m_frameRateNumerator;
			return m_timeBaseNumerator;
		}

		void setTimeBaseDenominator(uint32_t timeBaseDenominator)
		{
			m_timeBaseDenominator = timeBaseDenominator;
		}

		uint32_t timeBaseDenominator() const
		{
			if(!m_variableFrameRate)
				return m_frameRateDenominator;
			return m_timeBaseDenominator;
		}	

		void setTopFieldFirst()
		{
			m_interlaceMode = TopFieldFirst;
		}

		void setBottomFieldFirst()
		{
			m_interlaceMode = BottomFieldFirst;
		}

		void setMixedInterlacing()
		{
			m_interlaceMode = MixedInterlacing;
		}

		void setProgressive()
		{
			m_interlaceMode = Progressive;
		}

		void setAspectRatio(uint32_t aspectRatioWidth, uint32_t aspectRatioHeight)
		{
			m_aspectRatioWidth = aspectRatioWidth;
			m_aspectRatioHeight = aspectRatioHeight;
		}

		void setAspectRatioWidth(uint32_t aspectRatioWidth)
		{
			m_aspectRatioWidth = aspectRatioWidth;
		}

		uint32_t aspectRatioWidth() const
		{
			return m_aspectRatioWidth;
		}

		void setAspectRatioHeight(uint32_t aspectRatioHeight)
		{
			m_aspectRatioHeight = aspectRatioHeight;
		}

		uint32_t aspectRatioHeight() const
		{
			return m_aspectRatioHeight;
		}

		uint32_t lumaFrameSize() const
		{
			return m_width * m_height;
		}

		uint32_t chromaFrameSize() const
		{
			switch(m_colorSpace)
			{
				case Planar420:
					return lumaFrameSize() >> 2;
			}
			return 0;
		}

		uint32_t planes() const
		{
			switch(m_colorSpace)
			{
				case Planar420:
					return 3;
			}
			return 0;
		}

		uint32_t planeWidth(uint32_t plane)
		{
			switch(m_colorSpace)
			{
				case Planar420:
					if(plane == 0)
						return m_width;
					if(plane == 1 || plane == 2)
						return m_width >> 1;
					break;
			}
			return 0;
		}

		uint32_t planeHeight(uint32_t plane)
		{
			switch(m_colorSpace)
			{
				case Planar420:
					if(plane == 0)
						return m_height;
					if(plane == 1 || plane == 2)
						return m_height >> 1;
					break;
			}
			return 0;
		}

		uint32_t planeSize(uint32_t plane)
		{
			switch(m_colorSpace)
			{
				case Planar420:
					if(plane == 0)
						return lumaFrameSize();
					if(plane == 1 || plane == 2)
						return chromaFrameSize();
					break;
			}
			return 0;
		}

		bool interlaced() const
		{
			switch(m_interlaceMode)
			{
				case TopFieldFirst:
				case BottomFieldFirst:
				case MixedInterlacing:
					return true;
			}
			return false;
		}

		bool topFieldFirst() const
		{
			if(m_interlaceMode == TopFieldFirst)
				return true;
			return false;
		}	

		void setColorSpace(ColorSpace colorSpace)
		{
			m_colorSpace = colorSpace;
		}

		ColorSpace colorSpace() const
		{
			return m_colorSpace;
		}

		virtual class InputFrame *allocateFrame() = 0;

		virtual bool readFrame(InputFrame *result, uint64_t index) = 0;

	private:
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_frameRateNumerator;
		uint32_t m_frameRateDenominator;
		bool m_variableFrameRate;
		uint32_t m_timeBaseNumerator;
		uint32_t m_timeBaseDenominator;
		InterlaceMode m_interlaceMode;
		uint32_t m_aspectRatioWidth;
		uint32_t m_aspectRatioHeight;
		ColorSpace m_colorSpace;
	};
}
