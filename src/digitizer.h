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
};

#endif // DIGITIZER_H
