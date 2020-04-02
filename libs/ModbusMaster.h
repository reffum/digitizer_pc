/**
 * Modbus Master DLL
 * Oleg Gavrilchenko
 * reffum@bk.ru
 **/
#ifndef _MODBUS_MASTER_H_
#define _MODBUS_MASTER_H_

#include <cstdint>

#if defined(DLL_EXPORTS)
#define LIBSPEC	extern "C" __declspec(dllexport) 
#else
#define LIBSPEC	extern "C" __declspec(dllimport) 
#endif

/**
 * Error codes
 **/
#define MODBUS_MASTER_SUCCESS			0
#define MODBUS_MASTER_FAIL				1
#define MODBUS_MASTER_TCP_ERROR			2
#define MODBUS_MASTER_TIMEOUT_ERROR		3
#define MODBUS_MASTER_RESPONCE_ERROR	4
#define MODBUS_MASTER_INVALID_ARGUMENT	5


/**
 * Open TCP modbus connection. 
 * ip	-	ip address
 * port	-	TCP port
 * Return:	MODBUS_MASTER_SUCCESS
 *			MODBUS_MASTER_TCP_ERROR
 **/
LIBSPEC int ModbusTCP_Open(const char* ip, int port);

/**
 * Close TCP modbus connection
 * Return: MODBUS_MASTER_SUCCESS
 *			MODBUS_MASTER_FAIL
 **/
LIBSPEC int ModbusTCP_Close();

/**
 * Open COM-port modbus connection.
 * port		-	COM-port number
 * baudrate	-	COM-port baudrate
 * Return:	MODBUS_MASTER_SUCCESS
 *			MODBUS_MASTER_FAIL
 **/
LIBSPEC int ModbusCOM_Open(const int port, const int baudrate);

/**
 * Write single regiser.
 * id		Slave ID
 * addr		register address
 * value	register value
 * Return:	MODBUS_MASTER_SUCCESS
 *			MODBUS_MASTER_TCP_ERROR
 *			MODBUS_MASTER_TIMEOUT_ERROR
 *			< 0 Modbus exception with code by absolute value.
 **/
LIBSPEC int PUT(uint8_t id, uint16_t addr, uint16_t value);

/**
 * Read 1 register
 * id		Slave ID
 * addr		Registers start address
 * value	Register value
 * Return:	MODBUS_MASTER_SUCCESS
 *			MODBUS_MASTER_TCP_ERROR
 *			MODBUS_MASTER_TIMEOUT_ERROR
 *			< 0 - Modbus exception with code by absolute value.
 **/
LIBSPEC int GET(uint8_t id, uint16_t addr, uint16_t& value);

/**
* Read hold registers
* id		Slave ID
* addr		Registers start address
* count		count of read registers
* buffer	data buffer for registers value
* Return:	MODBUS_MASTER_SUCCESS
*			MODBUS_MASTER_TCP_ERROR
*			MODBUS_MASTER_TIMEOUT_ERROR
*			< 0 - Modbus exception with code by absolute value.
**/
LIBSPEC int TCP_ReadHold(uint8_t id, uint16_t addr, uint16_t count, uint16_t* buffer);

/**
 * Write multiple registers in Device 0
 * addr		Registers start address
 * count	count of write registers
 * buffer	data buffer for register value
 * Return:	MODBUS_MASTER_SUCCESS
 *			MODBUS_MASTER_TCP_ERROR
 *			< 0 - Modbus exception with code by absolute value.
 */
LIBSPEC int TCP_WriteMultipleDev0(uint16_t addr, uint16_t count, uint8_t* buffer);

/**
 * Modbus Encaplulated Interface Transport(2B) function.
 * id MODBUS slave ID. Broadcast request is not permitted.
 * req - data for send
 * req_size - req buffer size
 * resp - received data
 * resp_size - size of resp buffer
 * Return:	MODBUS_MASTER_SUCCESS
*			MODBUS_MASTER_TCP_ERROR
*			MODBUS_MASTER_TIMEOUT_ERROR
			< 0 - Modbus exception with code by absolute value.
 * **/
LIBSPEC int Modbus_Mei(uint8_t addr, const uint8_t* req, const size_t req_size,
	uint8_t* resp, size_t *resp_size);

/**
 * Set Modbus timeout
 * Timeout	timeout in ms(100ms..10s)
 * Return:  MODBUS_MASTER_SUCCESS
 *			MODBUS_MASTER_INVALID_ARGUMENT
 **/
LIBSPEC int TCP_SetTimeout(unsigned timeout);

/**
 * Get current Modbus timeout
 * Returen:	timeout in ms
 **/
LIBSPEC unsigned TCP_GetTimeout(void);

/**
* Close		modbus connection
* Return:	MODBUS_MASTER_SUCCESS
*			MODBUS_MASTER_FAIL
**/
LIBSPEC int Modbus_Close(void);

#endif /* _MODBUS_MASTER_H_ */
