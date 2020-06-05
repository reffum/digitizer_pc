#include <QNetworkDatagram>
#include <QFile>
#include "digitizer.h"
#include "digitizerexception.h"
#include "modbus.h"

const int TCP_DATA_PORT = 1024;
const int TCP_SIZE_PORT = 1025;


Digitizer::Digitizer(QObject* parent):
    m_dataSocket(nullptr), m_connectionState(false)
{
    Q_UNUSED(parent)

    m_dataSocket = new QTcpSocket(this);
    m_sizeSocket = new QTcpSocket(this);
}

Digitizer::~Digitizer()
{
    delete m_dataSocket;
}

void Digitizer::Connect(QString ip)
{
    try {
        Modbus::Connect(ip);

        quint16 id = Modbus::ReadRegister(ID);
        if(id != ID_VAL)
        {
            throw DigitizerException("ID устройства неверное");
        }

        m_dataSocket->connectToHost(QHostAddress(ip), TCP_DATA_PORT);
        m_sizeSocket->connectToHost(QHostAddress(ip), TCP_SIZE_PORT);
        m_connectionState = true;

    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

void Digitizer::Disconnect()
{
    Modbus::Disconnect();
    m_dataSocket->disconnectFromHost();
    m_sizeSocket->disconnectFromHost();
    m_connectionState = false;
}

void Digitizer::SetTestMode(bool b)
{
    quint16 reg = b ? _CR_TEST : 0;

    try {
        Modbus::WriteRegister(CR, reg);
    } catch (ModbusException e) {
        Disconnect();
        throw DigitizerException(e.getMessage());
    }
}

bool Digitizer::GetTestMode()
{
    try {
        quint16 reg = Modbus::ReadRegister(CR);

        if(reg & _CR_TEST)
            return true;
        else
            return false;
    } catch (ModbusException e) {
        Disconnect();
        throw DigitizerException(e.getMessage());
    }
}

void Digitizer::StartReceive(int size)
{
    try {
        Modbus::WriteRegister(DSIZE, static_cast<quint16>(size >> 6));

        quint16 cr = Modbus::ReadRegister(CR);
        cr |= _CR_START;
        Modbus::WriteRegister(CR, cr);

    } catch (ModbusException e) {
        Disconnect();
        throw DigitizerException(e.getMessage());
    }
}

QByteArray Digitizer::GetData()
{
    return m_dataSocket->readAll();
}

qint64 Digitizer::GetDataSize()
{
    return m_dataSocket->size();
}

bool Digitizer::GetConnectionState()
{
    return m_connectionState;
}

Version Digitizer::GetVersion()
{
    try {
        Version v;
        v.v1 = Modbus::ReadRegister(V1);
        v.v2 = Modbus::ReadRegister(V2);
        v.v3 = Modbus::ReadRegister(V3);

        return v;
    } catch (ModbusException e) {
        Disconnect();
        throw DigitizerException(e.getMessage());
    }
}

void Digitizer::AdcWrite(quint8 addr, quint8 data)
{
    try {
        Modbus::WriteRegister(ADC_ADDR, addr);
        Modbus::WriteRegister(ADC_DATA, data);
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

quint8 Digitizer::AdcRead(quint8 addr)
{
    try {
        Modbus::WriteRegister(ADC_ADDR, addr);
        quint16 data = Modbus::ReadRegister(ADC_DATA);
        return static_cast<quint8>(data);
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

void Digitizer::ClkdistWrite(quint8 addr, quint16 data)
{
    try {
        Modbus::WriteRegister(CLK_ADDR, addr);
        Modbus::WriteRegister(CLK_DATA, data);
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

quint16 Digitizer::ClkdistRead(quint8 addr)
{
    try {
        Modbus::WriteRegister(CLK_ADDR, addr);
        quint16 data = Modbus::ReadRegister(CLK_DATA);
        return static_cast<quint8>(data);
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

void Digitizer::SetPwmEnable(bool b)
{
    try {
        if(b)
            Modbus::WriteRegister(PWM_CONTROL, _PWM_CONTROL_ENABLE);
        else
            Modbus::WriteRegister(PWM_CONTROL, 0);
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

bool Digitizer::GetPwmEnable()
{
    try {
        quint16 pwmControl = Modbus::ReadRegister(PWM_CONTROL);
        if(pwmControl & _PWM_CONTROL_ENABLE)
            return true;
        else
            return false;
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

void Digitizer::SetPwmFreq(unsigned int freq)
{
    try {
        Modbus::WriteRegister(PWM_FREQ, static_cast<quint16>(freq));
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

unsigned Digitizer::GetPwmFreq()
{
    try {
        return Modbus::ReadRegister(PWM_FREQ);
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

void Digitizer::SetPwmDC(unsigned int dc)
{
    try {
        Modbus::WriteRegister(PWM_DC, static_cast<quint16>(dc));
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

unsigned Digitizer::GetPwmDC()
{
    try {
        return Modbus::ReadRegister(PWM_DC);
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

void Digitizer::SetDDSFreq(unsigned int freq)
{
    try {
        quint16 freq_h, freq_l;

        freq_h = (freq >> 16);
        freq_l = freq & 0xFFFF;

        Modbus::WriteRegister(DDS_FREQ_H, freq_h);
        Modbus::WriteRegister(DDS_FREQ_L, freq_l);
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

unsigned Digitizer::GetDDSFreq()
{
    try {
        unsigned freq, freq_l, freq_h;

        freq_h = Modbus::ReadRegister(DDS_FREQ_H);
        freq_l = Modbus::ReadRegister(DDS_FREQ_L);
        freq = (freq_h << 16) | freq_l;

        return freq;
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

void Digitizer::SetDDSAmp(unsigned int value)
{
    try {
        Modbus::WriteRegister(DDS_AMP, static_cast<quint16>(value));
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

unsigned Digitizer::GetDDSAmp()
{
    try {
        return Modbus::ReadRegister(DDS_AMP);
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

void Digitizer::WriteIoExpander(quint8 addr, quint8 data)
{
    try {
        quint16 word = data;
        word |= static_cast<quint16>(addr) << 8;

        Modbus::WriteRegister(IO_EXP_REG, word);
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

void Digitizer::RealTimeStart()
{
    stopRealTimeThread = false;

    realTimeThread = QThread::create(
                [=]()
    {
        const int WAIT_TIMEOUT = 1;

        // Size of next data packet
        quint32 lastPacketSize = 0;

        while(true)
        {
            // Wait for new packet size received
            while(true)
            {
                qint64 numBytes = m_sizeSocket->bytesAvailable();
                if(numBytes >= 4)
                {
                    break;
                }

                if(stopRealTimeThread)
                    return;

                QThread::sleep(WAIT_TIMEOUT);
            }

            // Read packet size
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wold-style-cast"
            m_sizeSocket->read((char*)&lastPacketSize, 4);
    #pragma GCC diagnostic pop


            // Receive data from data TCP connection
            while(true)
            {
                qint64 numBytes = m_dataSocket->bytesAvailable();
                if(numBytes >= lastPacketSize)
                {
                    break;
                }

                if(stopRealTimeThread)
                    return;
            }

            QByteArray data = m_dataSocket->readAll();

            // Save data to file
            QString fileName = SaveFilePath + QString("/") + QString("%1").arg(fileNum) + ".dat";
            QFile file(fileName);

            if( !file.open(QIODevice::ReadWrite | QIODevice::Truncate) )
            {
                emit saveFileError(QString("Ошибка при попытке открыть или создать файл %1").arg(fileName));
                return;
            }

            if(file.write(data) != data.size())
            {
                emit saveFileError(QString("Ошибка при записи данных в файл %1").arg(fileName));
                return;
            }

            fileNum++;
        }
    });

    try {
       Modbus::WriteRegister(CR, _CR_RT);
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }

    realTimeThread->start();
}

void Digitizer::RealTimeStop()
{
    try {
        stopRealTimeThread = true;
        realTimeThread->wait();

        Modbus::WriteRegister(CR, 0);
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

int Digitizer::RealTimeFrameNumber()
{
    return fileNum;
}
