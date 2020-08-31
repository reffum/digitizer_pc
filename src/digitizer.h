#ifndef DIGITIZER_H
#define DIGITIZER_H

#include <atomic>
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

    bool m_connectionState;

    // Stop-flags for real-time and no real-time receive data threads
    bool stopRealTimeThread = false;
	bool stopNoRealTimeThread = false;

    // Receive data threads
    QThread *realTimeThread;
	QThread* noRealTimeThread;

    // Directory for save real-time packets
    const QString SaveFilePath = "C:/Project/data";

	// IP address
	QString m_ip;

	// Received data size in no real-time mode
	size_t noRealTimeSize;

    // Data receive state
    enum ReceiveState {RECEIVE_NONE, RECEIVE_REAL_TIME, RECEIVE_NO_REAL_TIME};
    std::atomic<enum ReceiveState> m_receiveState;

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
    void PwmStart(unsigned N);

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

	// Start/stop no-realtime receive transsmit and receive 
	// size in 64 kB
	void StartReceive(int size);
	void StopReceive();

    void RealTimeStart();
    void RealTimeStop();

    int RealTimeFrameNumber();
    bool RealTimeOverflow();

	// Level sync control
	void LevelSyncEnable(bool);
	bool LevelSyncIsEnabled();

	void SetLevelSyncThr(quint16);
	quint16 GetLevelSyncThr();

	void SetLevelSyncN(quint16);
	quint16 GetLevelSyncN();

signals:
    // Emit in case of data receive or write to file error.
    void dataReceveError(QString msg);

    // Emit when no real-time data complete
    void noRealTimeDataReceiveComplete();
};

#endif // DIGITIZER_H
