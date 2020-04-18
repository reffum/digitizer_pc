#ifndef DIGITIZER_H
#define DIGITIZER_H

#include <QTcpSocket>
#include <QString>
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

    QTcpSocket *m_tcpSocket;
    bool m_connectionState;
    QByteArray m_byteArray;

public:
    Digitizer(QObject* parent = nullptr);
    virtual ~Digitizer();

    // In kB
    static const int MinDataSize = 64;

    static const unsigned PWM_MIN_FREQ = 10000;
    static const unsigned PWM_MAX_FREQ = 15000;
    static const unsigned PWM_MIN_DC = 1;
    static const unsigned PWM_MAX_DC = 99;

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

    // Write word to ADC
    void SendSpiWord(quint16);

    // Write word to CLKDIST
    void SendSpiClkDist(quint16);

    // PWM control
    void SetPwmEnable(bool);
    bool GetPwmEnable();

    void SetPwmFreq(unsigned freq);
    unsigned GetPwmFreq();

    void SetPwmDC(unsigned dc);
    unsigned GetPwmDC();
};

#endif // DIGITIZER_H
