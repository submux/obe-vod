#pragma once

#include <QtCore/QObject>

namespace MediaSource
{
	class MediaInput : public QObject
	{
	public:
		MediaInput(QObject *parent=0) : 
			QObject(parent)
		{
		}

		~MediaInput()
		{
		}
	};
}
