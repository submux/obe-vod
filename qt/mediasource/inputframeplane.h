#pragma once

#include <stdint.h>

namespace MediaSource
{
	class InputFramePlane
	{
	public:
		InputFramePlane(uint32_t width, uint32_t height) :
			m_width(width),
			m_height(height),
			m_data(0)
		{
		}

		~InputFramePlane()
		{
			delete [] m_data;
		}

		uint8_t *data() 
		{
			if(!m_data)
				m_data = new uint8_t[m_width * m_height];

			return m_data;
		}

		const uint8_t *data() const
		{
			return m_data;
		}

		uint32_t width() const
		{
			return m_width;
		}

		uint32_t height() const
		{
			return m_height;
		}

	private:
		uint32_t m_width;
		uint32_t m_height;
		uint8_t *m_data;
	};
}