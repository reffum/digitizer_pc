#include <string>
#include <QDebug>
#include <QFile>
#include "digitizer.h"
#include "digitizerexception.h"
#include "modbus.h"
#include "dataReceiver.h"

using namespace std;

const int TCP_DATA_PORT = 1024;

Digitizer::Digitizer(QObject* parent):
    m_connectionState(false), noRealTimeSize(0), noRealTimeThread(nullptr), 
    realTimeThread(nullptr)
{
    Q_UNUSED(parent);
    m_receiveState = RECEIVE_NONE;
}

Digitizer::~Digitizer()
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

		m_ip = ip;
        m_connectionState = true;

    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

void Digitizer::Disconnect()
{
    try {
        if (m_receiveState == RECEIVE_NO_REAL_TIME)
        {
            StopReceive();
        }
        else if (m_receiveState == RECEIVE_REAL_TIME)
        {
            RealTimeStop();
        }
        
        Modbus::Disconnect();

        m_connectionState = false;
    }
    catch (DigitizerException e)
    {
        qDebug() << "Digitizer::Disconnect() exceptin: " << e.GetErrorMessage();
    }
    
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

extern uint8_t ReceiveBuffer[];

QByteArray Digitizer::GetData()
{
	return QByteArray((char*)ReceiveBuffer, noRealTimeSize);
}

qint64 Digitizer::GetDataSize()
{
	return noRealTimeSize;
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

void Digitizer::StartReceive(int size)
{
    Q_ASSERT(m_receiveState == RECEIVE_NONE);

	noRealTimeSize = 0;

	noRealTimeThread = QThread::create(
		[=]()
		{
			try {
				string ipStr = m_ip.toStdString();
				ReceiveNoRealTimeData(ipStr.c_str(), TCP_DATA_PORT, size * 1024 * 1024, stopNoRealTimeThread, noRealTimeSize);
                emit noRealTimeDataReceiveComplete();
			}
			catch (exception e)
			{
				emit dataReceveError(e.what());
			}

            m_receiveState = RECEIVE_NONE;
		}
	);

	try {
		stopNoRealTimeThread = false;

        //TODO: Take out in separate method with separate button.
        m_receiveState = RECEIVE_NO_REAL_TIME;
		noRealTimeThread->start();

		Modbus::WriteRegister(DSIZE, static_cast<quint16>(size >> 6));
		quint16 cr = Modbus::ReadRegister(CR);

		cr |= _CR_START;
		Modbus::WriteRegister(CR, cr);        
	}
	catch (ModbusException e) {
        // Stop the thread
		stopNoRealTimeThread = true;
        noRealTimeThread->wait();

        Q_ASSERT(m_receiveState == RECEIVE_NONE);

        Disconnect();

		throw DigitizerException(e.getMessage());
	}
}

void Digitizer::StopReceive()
{
    if (m_receiveState == RECEIVE_NONE)
        return;

    Q_ASSERT(m_receiveState != RECEIVE_REAL_TIME);
    
    // Stop thread
	stopNoRealTimeThread = true;
	noRealTimeThread->wait();
    delete noRealTimeThread;
    noRealTimeSize = 0;
}

void Digitizer::RealTimeStart()
{
    stopRealTimeThread = false;

	realTimeThread = QThread::create(
		[=]()
		{
			try {
				string ipStr = m_ip.toStdString();
				fileNum = 0;
				ReceiveRealTimeData(ipStr.c_str(), TCP_DATA_PORT, stopRealTimeThread, fileNum);
			}
			catch (exception e)
			{
				emit dataReceveError(e.what());
			}

            // Disable real-time data send
            // Ignore possible MODBUS errors
            try {
                quint16 cr = Modbus::ReadRegister(CR);
                cr &= ~(_CR_RT);
                Modbus::WriteRegister(CR, cr);
            }
            catch (ModbusException e)
            {
                qDebug() << "Modbus Error in realTimeThread: " << e.getMessage();
            }
            
            m_receiveState = RECEIVE_NONE;
		}
	);

    try {
		Modbus::WriteRegister(CR, _CR_RT);
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }

    m_receiveState = RECEIVE_REAL_TIME;
    realTimeThread->start();
}

void Digitizer::RealTimeStop()
{
    if (m_receiveState != RECEIVE_REAL_TIME)
        return;

    try {    
        stopRealTimeThread = true;
        realTimeThread->wait();
        delete realTimeThread;

        Q_ASSERT(m_receiveState == RECEIVE_NONE);

    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}

int Digitizer::RealTimeFrameNumber()
{
    return fileNum;
}

bool Digitizer::RealTimeOverflow()
{
    try {
        return Modbus::ReadRegister(SR) & _SR_RT_OVF;
    } catch (ModbusException e) {
        throw DigitizerException(e.getMessage());
    }
}
