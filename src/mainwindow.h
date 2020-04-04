#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qindicator.h"

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

    QTimer *updateTimer;

    void Disconnect();

private slots:
    void on_connect_pushButton_clicked(bool checked = false);
    void on_start_pushButton_clicked(bool checked = false);
    void updateTimer_timeout();
    void on_save_action_triggered(bool checked = false);
};
#endif // MAINWINDOW_H
