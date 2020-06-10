#ifndef DIGITIZER_H
#define DIGITIZER_H

#include <QTcpSocket>
#include <QString>
#include <QThread>
#include "digitizerexception.h"

struct Version
{
    int v1,v2,v3;
};

//
// This class control Digitizer device
//
class Digitizer : public QObject
{
    Q_OBJECT

    QTcpSocket *m_dataSocket;
    QTcpSocket *m_sizeSocket;

    bool m_connectionState;
    QByteArray m_byteArray;

    bool stopRealTimeThread = false;
    QThread *realTimeThread;

    // Directory for save real-time packets
    const QString SaveFilePath = "C:/Project/data";

public:
    Digitizer(QObject* parent = nullptr);
    virtual ~Digitizer();

    // In kB
    static const int MinDataSize = 64;

    static const unsigned PWM_MIN_FREQ = 10000;
    static const unsigned PWM_MAX_FREQ = 15000;
    static const unsigned PWM_MIN_DC = 1;
    static const unsigned PWM_MAX_DC = 99;

    int fileNum = 1;

    // All methods throw DigitizerException in case of error, if otherwise not specified.
public:
    // Connect to device
    void Connect(QString IP);

    // Test mode
    bool GetTestMode();
    void SetTestMode(bool);

    // Start transsmit and receive packet
    // size in 64 kB
    void StartReceive(int size);

    // Disconnect from device
    void Disconnect();

    // Get data, received from device
    QByteArray GetData();

    // Get data size
    qint64 GetDataSize();

    bool GetConnectionState();

 	Version GetVersion();

    void AdcWrite(quint8 addr, quint8 data);
    quint8 AdcRead(quint8 addr);

    void ClkdistWrite(quint8 addr, quint16 data);
    quint16 ClkdistRead(quint8 addr);

    // PWM control
    void SetPwmEnable(bool);
    bool GetPwmEnable();

    void SetPwmFreq(unsigned freq);
    unsigned GetPwmFreq();

    void SetPwmDC(unsigned dc);
    unsigned GetPwmDC();

    // Set/Get frequency in Hz
    void SetDDSFreq(unsigned freq);
    unsigned GetDDSFreq();

    // Set/Get amplitude in mV
    void SetDDSAmp(unsigned value);
    unsigned GetDDSAmp();

    void WriteIoExpander(quint8 addr, quint8 data);

    void RealTimeStart();
    void RealTimeStop();
    int RealTimeFrameNumber();
    bool RealTimeOverflow();

signals:
    void saveFileError(QString msg);
};

#endif // DIGITIZER_H
