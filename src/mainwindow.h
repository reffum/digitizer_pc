#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "qindicator.h"
#include "digitizer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QIndicator *m_connectIndicator;
    QLabel *m_versionLabel;

    QTimer *updateTimer;

    Digitizer* m_digitizer;

    void Disconnect();

    void readSettings();
    void writeSettings();

    void closeEvent(QCloseEvent* event);

private slots:
	// Menu
	void on_save_action_triggered(bool checked = false);
	void on_fileParser_action_triggered(bool checked = false);
    void on_connect_pushButton_clicked(bool checked = false);
    void on_noRealTime_groupBox_toggled(bool);
    void on_realTime_groupBox_toggled(bool);
    void on_start_pushButton_clicked(bool checked = false);
    void on_save_pushButton_clicked(bool checked = false);
    void on_test_checkBox_stateChanged(int state);
    void updateTimer_timeout();    
    void on_adcSpi_pushButton_clicked(bool checked = false);
    void on_clkdistSpi_pushButton_clicked(bool checked = false);
    void on_pwm_pushButton_clicked(bool checked = false);
    void on_pwm_n_pushButton_clicked(bool checked = false);
    void on_pwm_checkBox_stateChanged(int state);
    void on_dds_pushButton_clicked(bool checked = false);
    void on_ioExp_pushButton_clicked(bool checked = false);
    void on_realTime_pushButton_clicked(bool checked = false);

	void on_levelSyncEnable_checkBox_stateChanged(int state);
	void on_levelSyncThr_spinBox_valueChanged(int i);
	void on_levelSyncN_spinBox_valueChanged(int i);
    void on_lvds_in_en_checkBox_stateChanged(int state);
    void on_lvds_in_start_pushButton_clicked(bool checked = false);
	
    void m_digitizer_saveFileError(QString msg);
    void m_digitizer_noRealTimeDataReceiveComplete();
};
#endif // MAINWINDOW_H
