#include "FileParseThread.h"
#include "dataReceiver.h"

using namespace std;

FileParseThread::FileParseThread(QString fileName, QString filePath, QObject* parent) :
	QThread(parent), m_fileName(fileName), m_filePath(filePath)
{
	m_progress = 0;
	m_stop = false;
}

void FileParseThread::run()
{
	try {
		string fileNameStr = m_fileName.toStdString();
		string filePathStr = m_filePath.toStdString();
		
		ParseDataFile(fileNameStr, filePathStr, m_progress, m_stop);
	}
	catch (exception e)
	{
		QString msg = e.what();
		emit errorOccured(e.what());
	}
}

void FileParseThread::stop()
{
	m_stop = true;
	wait();
}

double FileParseThread::progress()
{
	return m_progress;
}
