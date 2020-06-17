
//
// Receive data from board and save it in files in real-time mode
//
#include <WinSock2.h>
#include <Windows.h>
#include <assert.h>
#include <cstdint>
#include <exception>
#include <string>

#include "digitizer.h"

// link with Ws2_32.lib
#pragma comment(lib,"Ws2_32.lib")

using namespace std;

#define RECEIVE_BUFFER_SIZE	32*1024*1024

// Receive data buffer
uint8_t ReceiveBuffer[RECEIVE_BUFFER_SIZE];

const string FilePath = "C:/Project/data/";

void WriteDataToFile(string fileName, char* data, size_t size)
{
	int r;

	HANDLE hFile = CreateFileA(
		fileName.c_str(),
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, //FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		string msg;
		msg = "Creteate file" + fileName + " error. Last Error: " + to_string(GetLastError());
		throw exception(msg.c_str());
	}

	DWORD numberOfBytesWritten;

	r = WriteFile(
		hFile,
		data,
		size,
		&numberOfBytesWritten,
		NULL
	);

	if (!r)
	{
		string msg = "Write file " + fileName + " error. ERROR: " + to_string(GetLastError());
		CloseHandle(hFile);
		throw exception(msg.c_str());
	}

	if (numberOfBytesWritten != size)
	{
		string msg = "Write file " + fileName + " error. Number of written byte is not equal size of buffer.";
		CloseHandle(hFile);
		throw exception(msg.c_str());
	}

	CloseHandle(hFile);
}

void ReceiveRealTimeData(const char* ip, const short port, bool& stop, int& fileNum)
{
	// stop flag poll Timeout in us
	const int Timeout = 1000 * 500;

	WSADATA wsaData = { 0 };
	int iResult = 0;

	SOCKET sock = INVALID_SOCKET;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	assert(iResult == 0);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	assert(sock != INVALID_SOCKET);

	// Connect to the board
	sockaddr_in boardSockAddr;
	boardSockAddr.sin_family = AF_INET;
	boardSockAddr.sin_addr.s_addr = inet_addr(ip);
	boardSockAddr.sin_port = htons(port);

	try {
		iResult = connect(sock, (SOCKADDR*)&boardSockAddr, sizeof(boardSockAddr));
		if (iResult == SOCKET_ERROR)
		{
			throw exception("Connection error");
		}

		while (true)
		{

			// Receive size
			uint32_t packetSize, currentSize;

			// Wait for data or stop flag
			fd_set fd;
			FD_ZERO(&fd);
			FD_SET(sock, &fd);

			timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = Timeout;

			do {
				iResult = select(0, &fd, NULL, NULL, &tv);

				if (stop)
					goto close_connection;

			} while (iResult == 0);

			iResult = recv(sock, (char*)&packetSize, sizeof(packetSize), MSG_WAITALL);

			if (iResult == 0)
			{
				throw exception("Connection closed");
			}

			if (iResult == SOCKET_ERROR)
			{
				string msg = "Receive data error with code" + to_string(GetLastError());
				throw exception(msg.c_str());
			}

			if (packetSize == 0)
				continue;

			currentSize = packetSize;

			// Receive data
			while (true)
			{
				char* currentAddress = (char*)ReceiveBuffer;

				do {
					iResult = select(0, &fd, NULL, NULL, &tv);
					if (stop)
						goto close_connection;
				} while (iResult == 0);

				iResult = recv(sock, currentAddress, currentSize, 0);

				if (iResult == 0)
				{
					throw exception("Connection closed");
				}

				if (iResult == SOCKET_ERROR)
				{
					string msg = "Receive data error with code" + to_string(GetLastError());
					throw exception(msg.c_str());
				}

				currentSize -= iResult;

				if (currentSize == 0)
					break;

				currentAddress += iResult;
			}

			string fileName = FilePath + to_string(fileNum) + ".dat";
			//WriteDataToFile(fileName, (char*)ReceiveBuffer, packetSize);
			fileNum++;
		}

	close_connection:
		shutdown(sock, SD_BOTH);
		closesocket(sock);
		WSACleanup();
	}
	catch (exception e)
	{
		shutdown(sock, SD_BOTH);
		closesocket(sock);
		WSACleanup();

		throw;
	}
	
}

//
// Receive data in no real-time mode. Running in separate thread. Receive data and return.
// 
// ip, port - connection parameters
// size - setting size
// stop - stop flag. The thread stop receive and return when it become true
// actualSize - actual data received size
//
void ReceiveNoRealTimeData(const char *ip, const short port, size_t size, bool& stop, size_t& actualSize)
{
	// stop flag poll Timeout in us
	const int Timeout = 1000 * 500;

	WSADATA wsaData = { 0 };
	int iResult = 0;

	SOCKET sock = INVALID_SOCKET;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	assert(iResult == 0);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	assert(sock != INVALID_SOCKET);

	// Connect to the board
	sockaddr_in boardSockAddr;
	boardSockAddr.sin_family = AF_INET;
	boardSockAddr.sin_addr.s_addr = inet_addr(ip);
	boardSockAddr.sin_port = htons(port);

	try {
		// Wait for data or stop flag
		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(sock, &fd);

		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = Timeout;

		char* currentAddress = (char*)ReceiveBuffer;

		while (actualSize < size)
		{
			// Packet size
			uint32_t packetSize, currentSize;

			// Read packet size
			do {
				iResult = select(0, &fd, NULL, NULL, &tv);

				if (stop)
					goto close_connection;

			} while (iResult == 0);

			iResult = recv(sock, (char*)&packetSize, sizeof(packetSize), MSG_WAITALL);

			if (iResult == 0)
			{
				throw exception("Connection closed");
			}

			currentSize = packetSize;

			// Read packet
			while (true)
			{
				do {
					iResult = select(0, &fd, NULL, NULL, &tv);
					if (stop)
						goto close_connection;
				} while (iResult == 0);

				iResult = recv(sock, currentAddress, currentSize, 0);

				if (iResult == 0)
				{
					throw exception("Connection closed");
				}

				currentSize -= iResult;
				actualSize += iResult;

				currentAddress += iResult;

				if (currentSize == 0)
					break;
			}
		}
	

	close_connection:
		shutdown(sock, SD_BOTH);
		closesocket(sock);
		WSACleanup();

	}
	catch (exception e)
	{
		shutdown(sock, SD_BOTH);
		closesocket(sock);
		WSACleanup();

		throw;
	}
}
