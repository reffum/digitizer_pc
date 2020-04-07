#include <QNetworkDatagram>
#include "digitizer.h"
#include "digitizerexception.h"
#include "modbus.h"

const int TCP_DATA_PORT = 1024;

Digitizer::Digitizer(QObject* parent):
    m_tcpSocket(nullptr), m_connectionState(false)
{
    bool r;
    Q_UNUSED(parent)

    /* Create and connect to device for receive UDP data */
    m_tcpSocket = new QTcpSocket(this);
}

Digitizer::~Digitizer()
{
    delete m_tcpSocket;
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

        m_tcpSocket->connectToHost(QHostAddress(ip), TCP_DATA_PORT);
        connect(m_tcpSocket, &QTcpSocket::readyRead, this, &Digitizer::on_m_udpSocket_readyRead);

        m_connectionState = true;

    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

void Digitizer::Disconnect()
{
    Modbus::Disconnect();
    m_tcpSocket->disconnectFromHost();
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
    return m_tcpSocket->readAll();
}

qint64 Digitizer::GetDataSize()
{
    return m_tcpSocket->size();
}

bool Digitizer::GetConnectionState()
{
    return m_connectionState;
}

void Digitizer::on_m_udpSocket_readyRead()
{
    qDebug() << "Data received";    
}
