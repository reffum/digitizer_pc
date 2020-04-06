#include "digitizer.h"
#include "digitizerexception.h"
#include "modbus.h"

QUdpSocket* Digitizer::m_udpSocket;
bool Digitizer::m_connectionState = false;

Digitizer::Digitizer()
{

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

        /* Create and connect to device for receive UDP data */
        m_udpSocket = new QUdpSocket();
        m_udpSocket->connectToHost(ip, 1024);

        m_connectionState = true;

    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

void Digitizer::Disconnect()
{
    Modbus::Disconnect();
    m_udpSocket->disconnectFromHost();
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
        Modbus::WriteRegister(DSIZE, static_cast<quint16>(size));

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
    return m_udpSocket->readAll();
}

qint64 Digitizer::GetDataSize()
{
    return m_udpSocket->readBufferSize();
}

bool Digitizer::GetConnectionState()
{
    return m_connectionState;
}
