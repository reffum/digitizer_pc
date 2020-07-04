#include "modbus.h"
#include "ModbusMaster.h"

// Baudrate
const int ModbusBaudrate = 9600;

const int ModbusTimeout = 1000;

const int ModbusTcpPort = 502;

// Device Modbus ID
const quint8 ModbusId = 1;

// Number of read/write attempts
const int AttemptsNum = 3;

QMutex Modbus::mutex;

void Modbus::Connect(int comPort)
{
    int r = ModbusCOM_Open(comPort, ModbusBaudrate);

    if(r == MODBUS_MASTER_SUCCESS)
    {
        TCP_SetTimeout(ModbusTimeout);
        return;
    }

    throw ModbusException("Ошибка открытия порта");
}

void Modbus::Connect(QString ip)
{
    std::string s = ip.toStdString();
    const char* ipStr = s.c_str();
    int r = ModbusTCP_Open(ipStr, ModbusTcpPort);

    if(r == MODBUS_MASTER_SUCCESS)
    {
        TCP_SetTimeout(ModbusTimeout);
        return;
    }

    throw ModbusException("Ошибка TCP-подключения");
}

void Modbus::Disconnect()
{
    Modbus_Close();
}

quint16 Modbus::ReadRegister(quint16 addr)
{
    quint16 value;
    int r;
    int i;

	QMutexLocker locker(&mutex);

    for(i = 0; i < AttemptsNum; i++)
    {
        r = TCP_ReadHold(ModbusId, addr, 1, &value);
        if(r == MODBUS_MASTER_SUCCESS)
            break;
        else if(r == MODBUS_MASTER_TIMEOUT_ERROR)
            continue;
        else if(r < 0)
        {
            // Modbus Exception received
            throw ModbusException(
                        QString("Modbus Exception с кодом %1").arg(-r));
        }
    }

    if(i == AttemptsNum)
    {
        // Modbus Exception received
        throw ModbusException(
                    QString("MODBUS TIMEOUT при чтении регистра %1").arg(addr));
    }

    return value;
}

void Modbus::WriteRegister(quint16 addr, quint16 value)
{
    int r;
    int i;

	QMutexLocker locker(&mutex);

    for(i = 0; i < AttemptsNum; i++)
    {
        r = PUT(ModbusId, addr, value);
        if(r == MODBUS_MASTER_SUCCESS)
            break;
        else if(r == MODBUS_MASTER_TIMEOUT_ERROR)
            continue;
        else if(r < 0)
        {
            // Modbus Exception received
            throw ModbusException(
                        QString("Modbus Exception с кодом %1").arg(-r));
        }
    }

    if(i == AttemptsNum)
    {
        // Modbus Exception received
        throw ModbusException(
                    QString("MODBUS TIMEOUT при записи в регистр %1 значения %2")
                    .arg(addr)
                    .arg((value)));
    }
}
