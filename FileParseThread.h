#pragma once
#include <qthread.h>


class FileParseThread :
	public QThread
{
	Q_OBJECT

	QString m_fileName, m_filePath;
	double m_progress;
	bool m_stop;


	void run() override;

public:
	FileParseThread(QString fileName, QString filePath, QObject* parent = nullptr);

	void stop();
	double progress();

signals:
	void errorOccured(QString msg);
	void ready();
};
