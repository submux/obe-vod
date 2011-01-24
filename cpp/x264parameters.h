#pragma once

#include <stdint.h>

#include <x264.h>

class X264Parameters
{
public:
	X264Parameters()
	{
		x264_param_default(&m_parameters);
	}

	~X264Parameters()
	{
	}

	void setResolution(uint32_t width, uint32_t height)
	{
		m_parameters.i_width = width;
		m_parameters.i_height = height;
	}

	void setWidth(uint32_t width)
	{
		m_parameters.i_width = width;
	}

	uint32_t width() const
	{
		return m_parameters.i_width;
	}

	void setHeight(uint32_t height)
	{
		m_parameters.i_height = height;
	}

	uint32_t height() const
	{
		return m_parameters.i_height;
	}

	int setParameter(const char *name, const char *value)
	{
		return x264_param_parse(&m_parameters, name, value);
	}

	void setVariableFrameRateInput(bool variableFrameRateInput)
	{
		m_parameters.b_vfr_input = variableFrameRateInput ? 1 : 0;
	}

	bool variableFrameRateInput() const
	{
		return m_parameters.b_vfr_input ? true : false;
	}

	void setFrameRate(uint32_t frameRateNumerator, uint32_t frameRateDenominator)
	{
		m_parameters.i_fps_num = frameRateNumerator;
		m_parameters.i_fps_den = frameRateDenominator;
	}

	uint32_t frameRateNumerator() const
	{
		return m_parameters.i_fps_num;
	}

	uint32_t frameRateDenominator() const
	{
		return m_parameters.i_fps_den;
	}

	void setTimeBase(uint32_t timeBaseNumerator, uint32_t timeBaseDenominator)
	{
		m_parameters.i_timebase_num = timeBaseNumerator;
		m_parameters.i_timebase_den = timeBaseDenominator;
	}

	uint32_t timeBaseNumerator() const
	{
		return m_parameters.i_timebase_num;
	}

	uint32_t timeBaseDenominator() const
	{
		return m_parameters.i_timebase_den;
	}

	void setTopFieldFirst(bool topFieldFirst)
	{
		m_parameters.b_tff = topFieldFirst ? 1 : 0;
	}

	bool topFieldFirst() const
	{
		return m_parameters.b_tff ? true : false;
	}

	void setInterlaced(bool interlaced)
	{
		m_parameters.b_interlaced = interlaced ? 1 : 0;
	}

	bool interlaced() const
	{
		return m_parameters.b_interlaced ? true : false;
	}

	void setFrameCount(uint64_t frameCount)
	{
		m_parameters.i_frame_total = frameCount;
	}

	uint64_t frameCount() const
	{
		return m_parameters.i_frame_total;
	}

protected:
	friend class X264Encoder;

	x264_param_t *internalParameters()
	{
		return &m_parameters;
	}

private:
	x264_param_t m_parameters;
};
