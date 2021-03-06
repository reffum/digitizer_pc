#include <QFile>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <qsettings>
#include <QCloseEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "FileParserDialog.h"

const QString OrganizationName = "ITMO";
const QString ApplicationName = "Digitizer";

// Settings keys
const QString SettingsKeyRealTime = "RealTime";

// The path where file is saved, when user push button "Сохранить"
const QString DefaultSaveFile = "C:/Project/1.dat";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      m_digitizer(nullptr)
{
    ui->setupUi(this);
    setFixedSize(310, 880);

    m_connectIndicator = new QIndicator(this);
    m_connectIndicator->setColor(Qt::red);
    statusBar()->addWidget(m_connectIndicator);

    // Add tooltip for "Сохранить" button
    ui->save_pushButton->setToolTip(QString("Сохранить в файл %1").arg(DefaultSaveFile));

    ui->ovf_indicator->setColor(Qt::green);

    m_versionLabel = new QLabel(this);
    statusBar()->addWidget(m_versionLabel);

    updateTimer = new QTimer(this);
    updateTimer->setInterval(1000);

    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateTimer_timeout()));
    updateTimer->start();

    m_digitizer = new Digitizer(this);
    connect(m_digitizer, SIGNAL(dataReceveError(QString)), this, SLOT(m_digitizer_saveFileError(QString)));

    connect(m_digitizer, SIGNAL(noRealTimeDataReceiveComplete()),
        this, SLOT(m_digitizer_noRealTimeDataReceiveComplete()));


    readSettings();
}

void MainWindow::readSettings()
{
    QSettings settings(OrganizationName, ApplicationName);

    bool realTime = settings.value(SettingsKeyRealTime).toBool();

    QSignalBlocker b0(ui->noRealTime_groupBox);
    QSignalBlocker b1(ui->realTime_groupBox);

    if (!realTime)
    {
        ui->noRealTime_groupBox->setChecked(true);
        ui->realTime_groupBox->setChecked(false);
    }
    else
    {
        ui->noRealTime_groupBox->setChecked(false);
        ui->realTime_groupBox->setChecked(true);
    }
}

void MainWindow::writeSettings()
{
    QSettings settings(OrganizationName, ApplicationName);

    bool realTime = ui->realTime_groupBox->isChecked();

    settings.setValue(SettingsKeyRealTime, realTime);
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    writeSettings();

    m_digitizer->Disconnect();

    e->accept();
}

MainWindow::~MainWindow()
{
    delete m_digitizer;
    delete ui;
}

void MainWindow::Disconnect()
{
    ui->size_spinBox->setEnabled(false);
    ui->start_pushButton->setEnabled(false);
    ui->noRealTime_groupBox->setEnabled(false);
    ui->realTime_groupBox->setEnabled(false);
    m_connectIndicator->setColor(Qt::red);
    ui->save_action->setEnabled(false);
    ui->test_checkBox->setEnabled(false);
    ui->ioExp_pushButton->setEnabled(false);
    ui->adcSpi_pushButton->setEnabled(false);
    ui->clkdistSpi_pushButton->setEnabled(false);
    ui->pwm_groupBox->setEnabled(false);
    ui->dds_groupBox->setEnabled(false);
	ui->levelSync_groupBox->setEnabled(false);
    ui->connect_pushButton->setText("Подключиться");

    ui->realTime_pushButton->setChecked(true);
    ui->realTime_pushButton->setText("Старт");

    ui->lvds_in_groupBox->setEnabled(false);
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
            ui->noRealTime_groupBox->setEnabled(true);
            ui->realTime_groupBox->setEnabled(true);
            ui->test_checkBox->setEnabled(true);
            ui->save_action->setEnabled(true);
            ui->adcSpi_pushButton->setEnabled(true);
            ui->clkdistSpi_pushButton->setEnabled(true);
            ui->ioExp_pushButton->setEnabled(true);
            ui->pwm_groupBox->setEnabled(true);
            ui->dds_groupBox->setEnabled(true);
			ui->levelSync_groupBox->setEnabled(true);

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

            /* Get PWM parameters */
            QSignalBlocker pwm_checkBoxBlocker(ui->pwm_checkBox);

            if(m_digitizer->GetPwmEnable())
                ui->pwm_checkBox->setChecked(true);
            else
                ui->pwm_checkBox->setChecked(false);

            ui->pwmFreq_spinBox->setValue(static_cast<int>(m_digitizer->GetPwmFreq()));
            ui->pwmDC_spinBox->setValue(static_cast<int>(m_digitizer->GetPwmDC()));
            ui->ddsFreq_spinBox->setValue(static_cast<int>(m_digitizer->GetDDSFreq()));
            ui->ddsAmp_spinBox->setValue(static_cast<int>(m_digitizer->GetDDSAmp()));

            QSignalBlocker realTime_pushButtonBlocker(ui->realTime_pushButton);
            ui->realTime_pushButton->setChecked(false);
            ui->realTime_pushButton->setText("Старт");

			QSignalBlocker levelSyncBlocker(ui->levelSyncEnable_checkBox);
			QSignalBlocker levelSyncStartThrBlocker(ui->levelSyncThr_spinBox);
			QSignalBlocker levelSyncStopThrBlocker(ui->levelSyncN_spinBox);

			ui->levelSyncEnable_checkBox->setChecked(m_digitizer->LevelSyncIsEnabled() ? Qt::Checked : Qt::Unchecked);
			ui->levelSyncThr_spinBox->setValue(m_digitizer->GetLevelSyncThr());
			ui->levelSyncN_spinBox->setValue(m_digitizer->GetLevelSyncN());

            QSignalBlocker lvdsInEnBlocker(ui->lvds_in_en_checkBox);
            ui->lvds_in_groupBox->setEnabled(true);

            ui->lvds_in_en_checkBox->setChecked(m_digitizer->GetLvdsEn() ? Qt::Checked : Qt::Unchecked);
        }
        else
        {
            Disconnect();
            m_digitizer->Disconnect();
        }
    } catch (DigitizerException e) {
        QMessageBox::critical(this, "Ошибка подключения", e.GetErrorMessage());
        Disconnect();
        m_digitizer->Disconnect();
    }
}

void MainWindow::on_realTime_groupBox_toggled(bool on)
{
    QSignalBlocker blocker(ui->noRealTime_groupBox);

    if (on)
    {
        m_digitizer->StopReceive();
        ui->noRealTime_groupBox->setChecked(false);
    }
}

void MainWindow::on_noRealTime_groupBox_toggled(bool on)
{
    QSignalBlocker blocker(ui->realTime_groupBox);
    if (on)
    {
        m_digitizer->RealTimeStop();
        ui->noRealTime_groupBox->setChecked(true);
        ui->realTime_groupBox->setChecked(false);
    }
}

void MainWindow::on_start_pushButton_clicked(bool checked)
{
    if (checked)
    {

        try {
            int size = ui->size_spinBox->value();

            if (size < Digitizer::MinDataSize)
            {
                QMessageBox::warning(this, "Ошибка", "Размер данных должен быть больше 64 кБ");
                return;
            }

            size = size - (size % ui->size_spinBox->singleStep());
            ui->size_spinBox->setValue(size);

            m_digitizer->StartReceive(size);

            // Disable "Старт" button.
            ui->start_pushButton->setText("Стоп");
            ui->save_pushButton->setEnabled(false);
            ui->save_action->setEnabled(false);
        }
        catch (DigitizerException e) {
            QMessageBox::critical(this, "Ошибка", e.GetErrorMessage());
            Disconnect();
        }
    }
    else
    {
        m_digitizer->StopReceive();
        ui->save_pushButton->setEnabled(true);
        ui->save_action->setEnabled(true);
        ui->start_pushButton->setText("Старт");
    }
}

void MainWindow::m_digitizer_noRealTimeDataReceiveComplete()
{
    QSignalBlocker b(ui->start_pushButton);

    ui->start_pushButton->setChecked(false);
    ui->start_pushButton->setText("Старт");
    ui->save_pushButton->setEnabled(true);
    ui->save_action->setEnabled(true);
}

void MainWindow::on_save_pushButton_clicked(bool checked)
{
    Q_UNUSED(checked)

    QString fileName = DefaultSaveFile;

    QFile file(fileName);

    if(!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
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

void MainWindow::updateTimer_timeout()
{
    try {
        qint64 receiveSize = m_digitizer->GetDataSize();
        ui->receiveSize_lcdNumber->display(static_cast<int>(receiveSize));

        if (m_digitizer->GetConnectionState())
        {
            ui->rtFrames_lcdNumber->display(m_digitizer->RealTimeFrameNumber());

            if (m_digitizer->RealTimeOverflow())
                ui->ovf_indicator->setColor(Qt::red);
            else
                ui->ovf_indicator->setColor(Qt::green);
        }
    }
    catch (DigitizerException e)
    {
        QMessageBox::critical(this, "Ошибка", e.GetErrorMessage());
        Disconnect();
        m_digitizer->Disconnect();
    }
}

void MainWindow::on_save_action_triggered(bool checked)
{
    Q_UNUSED(checked)

    QString fileName = QFileDialog::getSaveFileName(this, "Укажите путь к файлу", QDir::homePath(), "Binary files (*.dat)");
    if(fileName.isNull())
        return;

    QFile file(fileName);

    if(!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
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

void MainWindow::on_fileParser_action_triggered(bool checked)
{
    Q_UNUSED(checked);

	FileParserDialog* dialog = new FileParserDialog(this);
	dialog->show();
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
       m_digitizer->AdcWrite(0, 0x66);
    } catch (DigitizerException e) {
        QMessageBox::critical(this,
                              "Ошибка",
                              QString("Ошибка при записи регистров АЦП(%1)").arg(e.GetErrorMessage()));
    }
}

void MainWindow::on_clkdistSpi_pushButton_clicked(bool checked)
{
    Q_UNUSED(checked)

    try {
       // m_digitizer->ClkdistWrite(2, 0xFF);
    } catch (DigitizerException e) {
        QMessageBox::critical(this,
                              "Ошибка",
                              QString("Ошибка при записи регистров CLKDIST(%1)").arg(e.GetErrorMessage()));
    }
}

void MainWindow::on_pwm_pushButton_clicked(bool checked)
{
    Q_UNUSED(checked)

    try {
        unsigned freq = static_cast<unsigned>(ui->pwmFreq_spinBox->value());
        unsigned dc = static_cast<unsigned>(ui->pwmDC_spinBox->value());

        m_digitizer->SetPwmFreq(freq);
        m_digitizer->SetPwmDC(dc);
    } catch (DigitizerException e) {
        QMessageBox::critical(this,
                              "Ошибка",
                              QString("Ошибка при установке параметров ШИМ(%1)").arg(e.GetErrorMessage()));

    }
}

void MainWindow::on_pwm_n_pushButton_clicked(bool checked)
{
    Q_UNUSED(checked)

    try{
        int N = ui->pwm_n_spinBox->value();
        m_digitizer->PwmStart(N);
    }
    catch (DigitizerException e) {
        QMessageBox::critical(this,
            "Ошибка",
            QString("Ошибка при запуске N-pulse ШИМ(%1)").arg(e.GetErrorMessage()));
    }
}

void MainWindow::on_pwm_checkBox_stateChanged(int state)
{
    try {
        if(state == Qt::Checked)
            m_digitizer->SetPwmEnable(true);
        else
            m_digitizer->SetPwmEnable(false);
    } catch (DigitizerException e) {
        QMessageBox::critical(this,
                              "Ошибка",
                              QString("Ошибка при установке параметров ШИМ(%1)").arg(e.GetErrorMessage()));

    }
}

void MainWindow::on_dds_pushButton_clicked(bool checked)
{
    Q_UNUSED(checked)

    try {
        unsigned freq = static_cast<unsigned>(ui->ddsFreq_spinBox->value());
        unsigned amp = static_cast<unsigned>(ui->ddsAmp_spinBox->value());

        m_digitizer->SetDDSFreq(freq);
        m_digitizer->SetDDSAmp(amp);

    } catch (DigitizerException e) {
        QMessageBox::critical(this,
                              "Ошибка",
                              QString("Ошибка при установке параметров DDS (%1)").arg(e.GetErrorMessage()));
    }
}

void MainWindow::on_ioExp_pushButton_clicked(bool checked)
{
    Q_UNUSED(checked)

    try {
        // Set all ports to outputs
        m_digitizer->WriteIoExpander(0x00, 0x00);
        m_digitizer->WriteIoExpander(0x01, 0x00);

        // Write to OLATA and OLATB registers
        m_digitizer->WriteIoExpander(0x14, 0xAA);
        m_digitizer->WriteIoExpander(0x15, 0xAA);
    } catch (DigitizerException e) {
        QMessageBox::critical(this,
                              "Ошибка",
                              QString("Ошибка при записи в I/O Expander (%1)").arg(e.GetErrorMessage()));
    }
}

void MainWindow::on_realTime_pushButton_clicked(bool checked)
{
    try {
        if(checked)
        {
            // Program in real-time receive mode
            m_digitizer->RealTimeStart();
            ui->realTime_pushButton->setText("Стоп");
        }
        else
        {
            m_digitizer->RealTimeStop();
            ui->realTime_pushButton->setText("Старт");
        }
    } catch (DigitizerException e) {
        QMessageBox::critical(this,
                              "Ошибка",
                              QString("Ошибка при установке режима Real-Time (%1)").arg(e.GetErrorMessage()));
    }
}

void MainWindow::m_digitizer_saveFileError(QString msg)
{
    // Error processing depend on current mode, no real-time or real-time

    if (ui->realTime_groupBox->isChecked())
    {
        // Real-time mode
        QSignalBlocker(ui->realTime_pushButton);

        QMessageBox::critical(this,
            "Ошибка",
            msg);

        ui->realTime_pushButton->setChecked(false);
        ui->realTime_pushButton->setText("Старт");
    }
    else
    {
        // No realtime mode
        QSignalBlocker b(ui->noRealTime_groupBox);

        QMessageBox::critical(this,
            "Ошибка",
            msg);

        ui->start_pushButton->setChecked(true);
        ui->start_pushButton->setText("Старт");
        ui->save_pushButton->setEnabled(true);
        ui->save_action->setEnabled(true);
    }
}

void MainWindow::on_levelSyncEnable_checkBox_stateChanged(int state)
{
	try {
		m_digitizer->LevelSyncEnable(state == Qt::Checked);
	}
	catch (DigitizerException e)
	{
		QMessageBox::critical(this,
			"Ошибка",
			QString("Ошибка при установке Синхронизации по уровню (%1)").arg(e.GetErrorMessage()));
	}
}
void MainWindow::on_levelSyncThr_spinBox_valueChanged(int i)
{
	try {
		m_digitizer->SetLevelSyncThr(i);
	}
	catch (DigitizerException e)
	{
		QMessageBox::critical(this,
			"Ошибка",
			QString("Ошибка при установке Синхронизации по уровню (%1)").arg(e.GetErrorMessage()));
	}
}

void MainWindow::on_levelSyncN_spinBox_valueChanged(int i)
{
	try {
		m_digitizer->SetLevelSyncN(i);
	}
	catch (DigitizerException e)
	{
		QMessageBox::critical(this,
			"Ошибка",
			QString("Ошибка при установке Синхронизации по уровню (%1)").arg(e.GetErrorMessage()));
	}
}

void MainWindow::on_lvds_in_en_checkBox_stateChanged(int state)
{
    try {
        m_digitizer->SetLvdsEn(state == Qt::Checked);
    }
    catch (DigitizerException e)
    {
        QMessageBox::critical(this,
            "Ошибка",
            QString("Ошибка при включении/отключении LVDS_IN %1").arg(e.GetErrorMessage()));
    }
}

void MainWindow::on_lvds_in_start_pushButton_clicked(bool checked)
{
    Q_UNUSED(checked)

    try {
        m_digitizer->LvdsStart();
    }
    catch (DigitizerException e)
    {
        QMessageBox::critical(this,
            "Ошибка",
            QString("Ошибка при запуске LVDS_IN %1").arg(e.GetErrorMessage()));
    }
}
