#ifndef MODBUS_H
#define MODBUS_H

#include <QString>

// Modbus registers
const quint16 ID = 0;
const quint16 CR = 1;
const quint16 SR = 2;
const quint16 DSIZE = 3;

const quint16 _CR_START = 1;
const quint16 _CR_TEST = 2;

// Exception for Modbus methods
class ModbusException
{
public:
    ModbusException(){}
    ModbusException(QString msg)
    {
        this->msg = msg;
    }

    ~ModbusException(){}

    QString getMessage()
    {
        return msg;
    }

private:
    QString msg;
};

class Modbus
{
private:
    Modbus(){}
public:
    static void Connect(int comPort);
    static void Connect(QString ip);
    static void Disconnect();
    static quint16 ReadRegister(quint16 addr);
    static void WriteRegister(quint16 addr, quint16 value);
};

#endif // MODBUS_H
