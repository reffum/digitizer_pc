#ifndef MODBUS_H
#define MODBUS_H

#include <QString>

// Modbus registers
const quint16 ID = 0;
const quint16 SR = 1;
const quint16 CR = 2;
const quint16 DSIZE = 3;
const quint16 REMOTE_DATA_PORT = 4;
const quint16 V1 = 5;
const quint16 V2 = 6;
const quint16 V3 = 7;
const quint16 ADC_ADDR = 8;
const quint16 ADC_DATA = 9;
const quint16 CLK_ADDR = 10;
const quint16 CLK_DATA = 11;
const quint16 PWM_FREQ = 12;
const quint16 PWM_DC = 13;
const quint16 PWM_CONTROL = 14;
const quint16 DDS_FREQ_H = 15;
const quint16 DDS_FREQ_L = 16;
const quint16 DDS_AMP = 17;
const quint16 IO_EXP_REG = 18;

const quint16 _CR_START = 1;
const quint16 _CR_TEST = 2;
const quint16 _CR_RT = 4;

const quint16 _SR_RT_OVF = 1;

const quint16 _PWM_CONTROL_ENABLE = 1;

const quint16 ID_VAL =0x55AA;

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
