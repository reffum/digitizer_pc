#include <qfiledialog.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include "FileParserDialog.h"
#include "ui_FileParserDialog.h"

FileParserDialog::FileParserDialog(QWidget* parent, Qt::WindowFlags f):
	QDialog(parent, f),
	ui(new Ui::FileParserDialog)
{
	ui->setupUi(this);
	setFixedSize(670, 140);
	updateTimer = new QTimer(this);
}

void FileParserDialog::on_file_pushButton_clicked(bool checked)
{
	Q_UNUSED(checked)

		m_fileName = QFileDialog::getOpenFileName(
			this,
			tr("Открыть файл"),
			QDir::homePath(),
			tr("Data file (*.dat)")
		);

	ui->file_lineEdit->setText(m_fileName);
}

void FileParserDialog::on_path_pushButton_clicked(bool checked)
{
	Q_UNUSED(checked)

	m_filePath = QFileDialog::getExistingDirectory(this,
									tr("Укажите каталок"),
									QDir::homePath()
								);
	ui->path_lineEdit->setText(m_filePath);
}

void FileParserDialog::on_start_pushButton_clicked(bool checked)
{
	Q_UNUSED(checked);

	m_fileName = ui->file_lineEdit->text();
	m_filePath = ui->path_lineEdit->text();

	m_thread = new FileParseThread(m_fileName, m_filePath, this);

	ui->start_pushButton->setEnabled(false);
	ui->stop_pushButton->setEnabled(true);
	ui->progressBar->setValue(0);

	connect(m_thread, SIGNAL(errorOccured(QString)), this, SLOT(m_thread_errorOccured(QString)));
	connect(m_thread, SIGNAL(finished()), this, SLOT(m_thread_finished()));
	connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateTimer_timeout));

	updateTimer->setInterval(1000);
	updateTimer->start();

	m_thread->start();
}

void FileParserDialog::on_stop_pushButton_clicked(bool checked)
{
	Q_UNUSED(checked);

	updateTimer->stop();
	m_thread->stop();
	
	disconnect(m_thread, SIGNAL(errorOccured()), this, SLOT(m_thread_errorOccured()));
	disconnect(m_thread, SIGNAL(finished()), this, SLOT(m_thread_finished()));

	delete m_thread;

	ui->start_pushButton->setEnabled(true);
	ui->stop_pushButton->setEnabled(false);
}

void FileParserDialog::m_thread_errorOccured(QString msg)
{
	m_thread->wait();
	updateTimer->stop();
	disconnect(m_thread, SIGNAL(errorOccured()), this, SLOT(m_thread_errorOccured()));
	disconnect(m_thread, SIGNAL(finished()), this, SLOT(m_thread_finished()));

	QString message = QString::fromLocal8Bit("Ошибка при разборе файла данных: ") + msg;

	QMessageBox::critical(this, QString::fromLocal8Bit("Ошибка"), message);

	ui->start_pushButton->setEnabled(true);
	ui->stop_pushButton->setEnabled(false);
}

void FileParserDialog::m_thread_finished()
{
	m_thread->wait();
	updateTimer->stop();

	disconnect(m_thread, SIGNAL(errorOccured()), this, SLOT(m_thread_errorOccured()));
	disconnect(m_thread, SIGNAL(finished()), this, SLOT(m_thread_finished()));

	QMessageBox::information(this, QString(), QString::fromLocal8Bit("Разбор файла окончен"));

	ui->start_pushButton->setEnabled(true);
	ui->stop_pushButton->setEnabled(false);
}

void FileParserDialog::updateTimer_timeout()
{
	// Update progress
	int p = m_thread->progress() * 100;

	ui->progressBar->setValue(p);
}
