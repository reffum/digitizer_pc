#include "digitizer.h"
#include "digitizerexception.h"
#include "modbus.h"

Digitizer::Digitizer(QObject* parent):
    m_udpSocket(nullptr), m_connectionState(false)
{
    Q_UNUSED(parent)

    /* Create and connect to device for receive UDP data */
    m_udpSocket = new QUdpSocket(this);
    m_udpSocket->bind(QHostAddress::LocalHost);

    connect(m_udpSocket, SIGNAL(readReady()), this, SLOT(on_m_udpSocket_readyRead));
}

Digitizer::~Digitizer()
{
    delete m_udpSocket;
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

        /* Write out UDP port to device */
        Modbus::WriteRegister(REMOTE_DATA_PORT, m_udpSocket->localPort());
        qDebug() << "Local port: " << m_udpSocket->localPort();

        m_connectionState = true;

    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

void Digitizer::Disconnect()
{
    Modbus::Disconnect();
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
    if(!m_udpSocket)
        return 0;

    return m_udpSocket->readBufferSize();
}

bool Digitizer::GetConnectionState()
{
    return m_connectionState;
}

void Digitizer::on_m_udpSocket_readyRead()
{
    qDebug() << "Data received";
}
