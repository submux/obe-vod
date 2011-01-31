#pragma once

#include <QtCore/QFile>
#include <QtCore/QString>

#include "videoinput.h"

namespace MediaSource
{
	class Y4MInput : public VideoInput
	{
	public:
		Y4MInput(QObject *parent=0);

		~Y4MInput();

		bool open(const class QString &fileName);

		void close();

		bool isOpen() const;

		bool seekFrame(uint64_t index);

		bool readFrame(class InputFrame *result, uint64_t index);

		class InputFrame *allocateFrame();

		uint64_t frameCount() const;

	private:
		QFile m_file;
	    
		int m_nextFrame;
		int m_sequenceHeaderLength;
		int m_frameHeaderLength;
		uint64_t m_frameSize;
		uint64_t m_frameCount;
	};
}
