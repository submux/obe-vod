#pragma once

#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>

#include "videoinput.h"

struct DtDeviceDesc;
struct DtHwFuncDesc;
class DtDevice;
class DtInpChannel;

namespace MediaSource
{
	class DektecDeviceDescription
	{
	public:
		qint64 serial() const;

	protected:
		friend class DektecFunctionDescription;

		DektecDeviceDescription(DtDeviceDesc *deviceDescription);

	private:
		DtDeviceDesc *m_deviceDescription;
	};

	class DektecFunctionDescription
	{
	public:

		bool canInput() const;

		bool canOutput() const;

		bool supportsSdi() const;

		bool supportsAsi() const;

		const DektecDeviceDescription *deviceDescription() const;

		~DektecFunctionDescription();

		int port() const;

	protected:
		friend class DektecSource;

		DektecFunctionDescription(DtHwFuncDesc *functionDescription);

	private:
		DtHwFuncDesc *m_functionDescription;
		DektecDeviceDescription *m_deviceDescription;
	};

	class DektecSource : public QThread
	{
	public:
		DektecSource(class DektecVideoInputSource *parent);

		~DektecSource();

		QList<DektecFunctionDescription *> &functions();

		bool connectTo(int functionIndex);

		bool go();

		class InputFrame *allocateFrame(quint32 width, quint32 height);

		InputFrame *convertFrame(const quint16 *frameIn, quint32 frameLength, bool topField);

	protected:
		void run();

	private:
		void enumerateFunctions();

		bool enableReceive();

		bool setReceiveIdle();

		void resetFIFO();

		bool waitCarrierDetect();

		int fifoLoad();

		bool waitFifoLoad(int requiredLevel);

	private:
		QList<DektecFunctionDescription *>m_functions;
		DtHwFuncDesc *m_internalFunctions;
		DtDevice *m_device;
		DtInpChannel *m_inputChannel;
		class DektecVideoInputSource *m_parent;
	};

	inline QList<DektecFunctionDescription *> &DektecSource::functions()
	{
		return m_functions;
	}

	class DektecVideoInputSource : public VideoInput
	{
	public:
		DektecVideoInputSource(QObject *parent=0);

		~DektecVideoInputSource();

		DektecSource &source();

		void start();		

		void stop();

		class InputFrame *allocateFrame();

		bool readFrame(InputFrame *result, uint64_t index);

		void pushFrame(class InputFrame *frame);

	private:
		QMutex m_queueMutex;
		QWaitCondition m_framePresentCondition;
		QList<class InputFrame *>m_frameQueue;
		DektecSource m_source;
	};
}
