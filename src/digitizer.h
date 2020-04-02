#ifndef DIGITIZER_H
#define DIGITIZER_H

#include <QUdpSocket>
#include <QString>
#include "digitizerexception.h"

//
// This class control Digitizer device
//
class Digitizer
{
private:
    Digitizer();        

    static QUdpSocket *m_udpSocket; 

    static bool m_connectionState;

public:
    // In kB
    static const int MinDataSize = 64;

    // All methods throw DigitizerException in case of error, if otherwise not specified.
public:
    // Connect to device
    static void Connect(QString IP);

    // Test mode
    static bool GetTestMode();
    static void SetTestMode(bool);

    // Start transsmit and receive packet
    // size in 64 kB
    static void StartReceive(int size);

    // Disconnect from device
    static void Disconnect();

    // Get data, received from device
    static QByteArray GetData();

    // Get data size
    static qint64 GetDataSize();

    static bool GetConnectionState();
};

#endif // DIGITIZER_H
