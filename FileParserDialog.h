#pragma once
#include <qdialog.h>
#include "FileParseThread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FileParserDialog; }
QT_END_NAMESPACE

class FileParserDialog :
	public QDialog
{
	Q_OBJECT

	Ui::FileParserDialog *ui;

	// Data file
	QString m_fileName;

	// Frame files path
	QString m_filePath;

	FileParseThread* m_thread;
	QTimer* updateTimer;

public:
	FileParserDialog(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

private slots:
	void on_file_pushButton_clicked(bool checked = false);
	void on_path_pushButton_clicked(bool checked = false);
	void on_start_pushButton_clicked(bool checked = false);
	void on_stop_pushButton_clicked(bool checked = false);
	void m_thread_errorOccured(QString msg);
	void m_thread_ready(void);
	void updateTimer_timeout(void);
};

