#include <QFile>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "digitizer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(300, 350);

    m_connectIndicator = new QIndicator(this);
    m_connectIndicator->setColor(Qt::red);
    statusBar()->addWidget(m_connectIndicator);

    updateTimer = new QTimer(this);
    updateTimer->setInterval(1000);

    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateTimer_timeout()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::Disconnect()
{
    ui->size_spinBox->setEnabled(false);
    ui->start_pushButton->setEnabled(false);
    m_connectIndicator->setColor(Qt::red);
    ui->save_action->setEnabled(false);
    ui->test_checkBox->setEnabled(false);
    ui->connect_pushButton->setText("Подключиться");
}

void MainWindow::on_connect_pushButton_clicked(bool checked)
{
    Q_UNUSED(checked)

    try {
        if(!Digitizer::GetConnectionState())
        {
            QString ip = ui->ip_lineEdit->text();
            Digitizer::Connect(ip);

            ui->size_spinBox->setEnabled(true);
            ui->start_pushButton->setEnabled(true);
            ui->test_checkBox->setEnabled(true);

            ui->connect_pushButton->setText("Отключиться");
            m_connectIndicator->setColor(Qt::green);
        }
        else
        {
            Disconnect();
        }
    } catch (DigitizerException e) {
        QMessageBox::critical(this, "Ошибка подключения", e.GetErrorMessage());
    }
}

void MainWindow::on_start_pushButton_clicked(bool checked)
{
    Q_UNUSED(checked)

    try {
        int size = ui->size_spinBox->value();

        if(size < Digitizer::MinDataSize)
        {
            QMessageBox::warning(this, "Ошибка", "Размер данных должен быть больше 64 кБ");
            return;
        }

        size = size - (size % ui->size_spinBox->singleStep());
        ui->size_spinBox->setValue(size);

        Digitizer::StartReceive(size);

    } catch (DigitizerException e) {
        QMessageBox::critical(this, "Ошибка", e.GetErrorMessage());
        Disconnect();
    }
}

void MainWindow::updateTimer_timeout()
{
    qint64 receiveSize = Digitizer::GetDataSize();
    ui->receiveSize_lcdNumber->display(static_cast<int>(receiveSize));
}

void MainWindow::on_save_action_triggered(bool checked)
{
    Q_UNUSED(checked)

    QString fileName = QFileDialog::getSaveFileName(this, "Укажите путь к файлу", QDir::homePath(), "Binary files (*.dat)");
    if(fileName.isNull())
        return;

    QFile file(fileName);

    if(!file.open(QIODevice::ReadWrite))
    {
        QMessageBox::critical(this, "Ошибка", "Невозможно создать файл");
        return;
    }

    QByteArray byteArray = Digitizer::GetData();

    qint64 size = file.write(byteArray);

    if(size != byteArray.size())
    {
        QMessageBox::critical(this, "Ошибка", "Ошибка при записи данных в файл");
    }
}

void MainWindow::on_test_checkBox_stateChanged(int state)
{
    try {
        if(state == Qt::Checked)
        {
            Digitizer::SetTestMode(true);
        }
        else
        {
            Digitizer::SetTestMode(false);
        }
    } catch (DigitizerException e) {
        QMessageBox::critical(this,
                              "Ошибка",
                              QString("Ошибка при установке тестового режима: %1")
                              .arg(e.GetErrorMessage()));
    }

}
