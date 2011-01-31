#include "dektecsource.h"
#include "inputframe.h"
#include "inputframeplane.h"

#include <QtCore/QDebug>

#define _USE_DTAPIDLL
#define WIN32_LEAN_AND_MEAN
#include "DekTec/dtapi.h"

namespace MediaSource
{
	DektecDeviceDescription::DektecDeviceDescription(DtDeviceDesc *deviceDescription) :
		m_deviceDescription(deviceDescription)
	{
	}

	qint64 DektecDeviceDescription::serial() const
	{
		return m_deviceDescription->m_Serial;
	}

	DektecFunctionDescription::DektecFunctionDescription(DtHwFuncDesc *functionDescription) : 
		m_functionDescription(functionDescription)
	{
		m_deviceDescription = new DektecDeviceDescription(&functionDescription->m_DvcDesc);
	}

	DektecFunctionDescription::~DektecFunctionDescription()
	{
		delete m_deviceDescription;
	}

	bool DektecFunctionDescription::canInput() const
	{
		return (m_functionDescription->m_ChanType & DTAPI_CHAN_INPUT) ? true : false;
	}

	bool DektecFunctionDescription::canOutput() const
	{
		return (m_functionDescription->m_ChanType & DTAPI_CHAN_OUTPUT) ? true : false;
	}

	bool DektecFunctionDescription::supportsSdi() const
	{
		return (m_functionDescription->m_Flags & DTAPI_CAP_SDI) ? true : false;
	}

	bool DektecFunctionDescription::supportsAsi() const
	{
		return (m_functionDescription->m_Flags & DTAPI_CAP_ASI) ? true : false;
	}

	const DektecDeviceDescription *DektecFunctionDescription::deviceDescription() const
	{
		return m_deviceDescription;
	}

	int DektecFunctionDescription::port() const
	{
		return m_functionDescription->m_Port;
	}

	DektecSource::DektecSource(DektecVideoInputSource *parent) : 
		QThread(parent),
		m_parent(parent)
	{
		enumerateFunctions();

		m_device = new DtDevice;
		m_inputChannel = new DtInpChannel;
	}

	void DektecSource::enumerateFunctions()
	{
		m_internalFunctions = new DtHwFuncDesc[20];

		int functionCount;
		
		DTAPI_RESULT dr = ::DtapiHwFuncScan(20, functionCount, m_internalFunctions);
		if(dr != DTAPI_OK)
			return;

		for(int i=0; i<functionCount; i++)
			m_functions.append(new DektecFunctionDescription(&m_internalFunctions[i]));
	}

	DektecSource::~DektecSource()
	{
		delete m_inputChannel;
		delete m_device;
		delete [] m_internalFunctions;
	}

	bool DektecSource::connectTo(int functionIndex)
	{
		if(functionIndex >= m_functions.count())
		{
			qDebug() << "functionIndex is invalid";
			return false;
		}

		DektecFunctionDescription *function = m_functions[functionIndex];

		qint64 serial = function->deviceDescription()->serial();
		if(serial == -1)
			return false;

		DTAPI_RESULT dr = m_device->AttachToSerial(serial);		
		if(dr != DTAPI_OK)
		{
			qDebug() << "Failed to attach to device with serial " << serial;
			return false;
		}
		
		int port = function->port();
		dr = m_inputChannel->AttachToPort(m_device, port);
		if(dr != DTAPI_OK)
		{
			qDebug() << "Failed to attach to port " << port << " on device with serial " << serial;
			qDebug() << DtapiResult2Str(dr);
			return false;
		}

		dr = m_inputChannel->SetRxControl(DTAPI_RXCTRL_RCV);
		if(dr != DTAPI_OK)
		{
			qDebug() << "Failed to set port " << port << " on device with serial " << serial << " to input mode";
			qDebug() << DtapiResult2Str(dr);
			return false;
		}

		if(!setReceiveIdle())
			return false;

		resetFIFO();

		dr = m_inputChannel->SetRxMode(DTAPI_RXMODE_SDI_FULL | DTAPI_RXMODE_SDI_10B);
		if(dr != DTAPI_OK)
		{
			qDebug() << "Failed to set port " << port << " on device with serial " << serial << " to input mode";
			qDebug() << DtapiResult2Str(dr);
			return false;
		}

		if(!waitCarrierDetect())
			return false;

		return true;
	}

	bool DektecSource::enableReceive()
	{
		DTAPI_RESULT result = m_inputChannel->SetRxControl(DTAPI_RXCTRL_RCV);
		if(result != DTAPI_OK)
		{
			qDebug() << ::DtapiResult2Str(result);
			return false;
		}

		return true;
	}

	bool DektecSource::setReceiveIdle()
	{
		if(m_inputChannel->SetRxControl(DTAPI_RXCTRL_IDLE) == DTAPI_OK)
			return true;

		return false;
	}

	void DektecSource::resetFIFO()
	{
		m_inputChannel->ClearFifo();
		m_inputChannel->ClearFlags(0xFFFFFFFF);
	}

	bool DektecSource::waitCarrierDetect()
	{
		bool validSignal = false;
		while(!validSignal)
		{
			int carrierDetect;
			int sdiMode = -1;
			int n;

			m_inputChannel->GetStatus(sdiMode, n, carrierDetect, n, n, n);
			if(carrierDetect == DTAPI_CLKDET_OK)
				validSignal = true;
			else
				Sleep(500);
		}

		return validSignal;
	}

	int DektecSource::fifoLoad()
	{
		int fifoLoad;
		if(m_inputChannel->GetFifoLoad(fifoLoad) != DTAPI_OK)
			return -1;
		return fifoLoad;
	}

	bool DektecSource::waitFifoLoad(int requiredLevel)
	{
		int load = fifoLoad();
		while(load >= 0 && load < requiredLevel)
		{
			Sleep(0);
			load = fifoLoad();
		}
		if(load == -1)
			return false;
		return true;
	}

	bool DektecSource::go()
	{
		if(!enableReceive())
			return false;

		start();

		return true;
	}

	InputFrame *DektecSource::allocateFrame(quint32 width, quint32 height)
	{
		InputFrame *newFrame = new InputFrame;
		newFrame->setColorSpace(VideoInput::Planar420);
		newFrame->addPlane(width, height);
		newFrame->addPlane(width >> 1, height >> 1);
		newFrame->addPlane(width >> 1, height >> 1);
		return newFrame;
	}

	InputFrame *DektecSource::convertFrame(const quint16 *frameIn, quint32 frameLength, bool topField)
	{
		InputFrame *newFrame = allocateFrame(720, 288);

		quint8 *outY = newFrame->plane(0)->data();
		quint8 *outCb = newFrame->plane(1)->data();
		quint8 *outCr = newFrame->plane(2)->data();

		quint32 lineOffset = 335;
		if(topField)
			lineOffset = 23;

		for(int y=0; y<288; y++)
		{
			const quint16 *in = frameIn + ((y + lineOffset) * 1728) + (1728 - 1440);

			if(y & 1)
			{
				for(int x=0; x<720; x+=2)
				{
					in++;
					*outY++ = (*in++) >> 2;
					in++;
					*outY++ = (*in++) >> 2;					
				}
			}
			else
			{
				for(int x=0; x<720; x+=2)
				{
					*outCb++ = (*in++) >> 2;
					*outY++ = (*in++) >> 2;
					*outCr++ = (*in++) >> 2;
					*outY++ = (*in++) >> 2;					
				}
			}
		}

		return newFrame;
	}

	void DektecSource::run()
	{
		const int MaximumFrameSize = 1728 *		// Width of an SDI frame
									 625 * 		// Height of an SDI frame
									 10 /		// 10 bits in a pixel
									 8;			// 8 bits in a bytes

		quint8 *frameBuffer = new quint8[MaximumFrameSize];
		quint16 *decodedBuffer = new quint16[1728 * 625];

		quint64 fieldNumber = 0;

		while(true)
		{
			if(!waitFifoLoad(MaximumFrameSize))
				break;

			int bytesRead = MaximumFrameSize;
			DTAPI_RESULT result = m_inputChannel->ReadFrame((unsigned int *) frameBuffer, bytesRead, 200);
			if(result != DTAPI_OK)
				break;

			if(bytesRead == 0)
			{
				Sleep(10);
				continue;
			}
			
			quint16 *out = decodedBuffer;

			quint32 *in = (quint32 *) frameBuffer;
			quint32 *end = in + (bytesRead >> 2);
			while(in < end)
			{
				quint32  a = *in++;
				*out++ = (quint16)a & 0x3ff;
				*out++ = (quint16)(a >> 10) & 0x3ff;
				*out++ = (quint16)(a >> 20) & 0x3ff;
				quint32  b = *in++;
				*out++ = (quint16)((a >> 30) | (b << 2)) & 0x3ff;
				*out++ = (quint16)(b >> 8) & 0x3ff;
				*out++ = (quint16)(b >> 18) & 0x3ff;
				quint32  c = *in++;
				*out++ = (quint16)((b >> 28) | (c << 4)) & 0x3ff;
				*out++ = (quint16)(c >> 6) & 0x3ff;
				*out++ = (quint16)(c >> 16) & 0x3ff;
				quint32  d = *in++;
				*out++ = (quint16)((c >> 26) | (d << 6)) & 0x3ff;
				*out++ = (quint16)(d >> 4) & 0x3ff;
				*out++ = (quint16)(d >> 14) & 0x3ff;
				quint32  e = *in++;
				*out++ = (quint16)((d >> 24) | (e << 8)) & 0x3ff;
				*out++ = (quint16)(e >> 2) & 0x3ff;
				*out++ = (quint16)(e >> 12) & 0x3ff;
				*out++ = (quint16)(e >> 22);
			}

			quint32 pixelsConverted = out - decodedBuffer;

			InputFrame *newFrame = convertFrame(decodedBuffer, pixelsConverted, true);
			newFrame->setPresentationTimeStamp(fieldNumber * 90000 / 50);
			fieldNumber++;
			m_parent->pushFrame(newFrame);

			newFrame = convertFrame(decodedBuffer, pixelsConverted, false);
			newFrame->setPresentationTimeStamp(fieldNumber * 90000 / 50);
			fieldNumber++;
			m_parent->pushFrame(newFrame);

			qDebug() << bytesRead;
		}

		delete [] frameBuffer;
		delete [] decodedBuffer;
	}

	DektecVideoInputSource::DektecVideoInputSource(QObject *parent) :
		VideoInput(parent),
		m_source(this)
	{
		setWidth(720);
		setHeight(288);
		setFrameRate(50, 1);
		setTimeBase(50, 1);
		setTopFieldFirst();
		setColorSpace(VideoInput::Planar420);
	}

	DektecVideoInputSource::~DektecVideoInputSource()
	{
	}

	DektecSource &DektecVideoInputSource::source()
	{
		return m_source;
	}

	void DektecVideoInputSource::start()
	{
		m_source.go();
	}

	void DektecVideoInputSource::stop()
	{
	}

	InputFrame *DektecVideoInputSource::allocateFrame()
	{
		return new InputFrame;
	}

	bool DektecVideoInputSource::readFrame(InputFrame *result, uint64_t index)
	{
		m_queueMutex.lock();
		if(m_frameQueue.isEmpty())
		{
			if(!m_framePresentCondition.wait(&m_queueMutex))
				return false;
		}
		InputFrame *nextFrame = m_frameQueue.takeFirst();
		m_queueMutex.unlock();

		result->take(nextFrame);
		delete nextFrame;
		return true;
	}

	void DektecVideoInputSource::pushFrame(class InputFrame *frame)
	{
		m_queueMutex.lock();
		m_frameQueue.append(frame);
		m_queueMutex.unlock();
		m_framePresentCondition.wakeOne();
	}
}

