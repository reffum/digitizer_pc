#include <QFile>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      m_digitizer(nullptr)
{
    ui->setupUi(this);
    setFixedSize(300, 350);

    m_connectIndicator = new QIndicator(this);
    m_connectIndicator->setColor(Qt::red);
    statusBar()->addWidget(m_connectIndicator);

    m_versionLabel = new QLabel(this);
    statusBar()->addWidget(m_versionLabel);

    updateTimer = new QTimer(this);
    updateTimer->setInterval(1000);

    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateTimer_timeout()));
    updateTimer->start();

    m_digitizer = new Digitizer(this);
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
    ui->adcSpi_pushButton->setEnabled(false);
    ui->clkdistSpi_pushButton->setEnabled(false);
    ui->connect_pushButton->setText("Подключиться");
}

void MainWindow::on_connect_pushButton_clicked(bool checked)
{
    Q_UNUSED(checked)

    try {
        if(!m_digitizer->GetConnectionState())
        {
            QString ip = ui->ip_lineEdit->text();
            m_digitizer->Connect(ip);

            ui->size_spinBox->setEnabled(true);
            ui->start_pushButton->setEnabled(true);
            ui->test_checkBox->setEnabled(true);
            ui->save_action->setEnabled(true);
            ui->adcSpi_pushButton->setEnabled(true);
            ui->clkdistSpi_pushButton->setEnabled(true);

            ui->connect_pushButton->setText("Отключиться");
            m_connectIndicator->setColor(Qt::green);

            Version v = m_digitizer->GetVersion();

            m_versionLabel->setText(QString("Версия прошивки: %1.%2.%3")
                                    .arg(v.v1)
                                    .arg(v.v2)
                                    .arg(v.v3));
            /* Get state of TEST mode */
            bool test = m_digitizer->GetTestMode();

            ui->test_checkBox->setCheckState(test ? Qt::Checked : Qt::Unchecked);
        }
        else
        {
            Disconnect();
            m_digitizer->Disconnect();
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

        m_digitizer->StartReceive(size);

    } catch (DigitizerException e) {
        QMessageBox::critical(this, "Ошибка", e.GetErrorMessage());
        Disconnect();
    }
}

void MainWindow::updateTimer_timeout()
{
    qint64 receiveSize = m_digitizer->GetDataSize();
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

    QByteArray byteArray = m_digitizer->GetData();

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
            m_digitizer->SetTestMode(true);
        }
        else
        {
            m_digitizer->SetTestMode(false);
        }
    } catch (DigitizerException e) {
        QMessageBox::critical(this,
                              "Ошибка",
                              QString("Ошибка при установке тестового режима: %1")
                              .arg(e.GetErrorMessage()));
    }

}

// This method write words in SPI, when user click to "Write to ADC" button
void MainWindow::on_adcSpi_pushButton_clicked(bool checked)
{
    Q_UNUSED(checked)

    try {
        m_digitizer->SendSpiWord(0x000E0);
        m_digitizer->SendSpiWord(0x000E0);
    } catch (DigitizerException e) {
        QMessageBox::critical(this,
                              "Ошибка",
                              QString("Ошибка при записи регистров АЦП(%1)").arg(e.GetErrorMessage()));
    }
}

/* Return CLKDIST word(see datasheet for details) */
static quint16 getClkDistWord(quint16 data, quint8 addr)
{
    quint16 w = 0;
    w = static_cast<quint16>(data << 7);

    w |= (addr & 0x7) << 3;

    w |= 0x1;

    return w;
}

void MainWindow::on_clkdistSpi_pushButton_clicked(bool checked)
{
    Q_UNUSED(checked)

    try {
        m_digitizer->SendSpiClkDist(getClkDistWord(0, 4));
    } catch (DigitizerException e) {
        QMessageBox::critical(this,
                              "Ошибка",
                              QString("Ошибка при записи регистров CLKDIST(%1)").arg(e.GetErrorMessage()));
    }
}
