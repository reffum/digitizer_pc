
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

const string DataFile = "C:/Project/data/data.dat";

size_t getFileSize(string filename)
{
	HANDLE hFile = CreateFileA(
		filename.c_str(),
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		throw exception("Can't open binary file");
	}

	LARGE_INTEGER liSize;

	if (!GetFileSizeEx(hFile, &liSize))
	{
		CloseHandle(hFile);
		throw exception("Can't get binary file size");
	}

	CloseHandle(hFile);

	size_t size = (size_t)liSize.QuadPart;

	return size;
}


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

static HANDLE CreateDataFile(string fileName)
{
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

	return hFile;
}

static void WriteDataToFile(HANDLE hFile, char* data, size_t size)
{
	int r;

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
		string msg = "Write file error. ERROR: " + to_string(GetLastError());
		throw exception(msg.c_str());
	}

	if (numberOfBytesWritten != size)
	{
		string msg = "Write file error. Number of written byte is not equal size of buffer.";
		throw exception(msg.c_str());
	}
}

static void CloseDataFile(HANDLE hFile)
{
	CloseHandle(hFile);
}

void ReceiveRealTimeData(const char* ip, const short port, bool& stop, int& fileNum)
{
	// stop flag poll Timeout in us
	const int Timeout = 1000 * 500;

	HANDLE hFile;
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
		hFile = CreateDataFile(DataFile);

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


			do {
				fd_set fd;
				FD_ZERO(&fd);
				FD_SET(sock, &fd);

				timeval tv;
				tv.tv_sec = 0;
				tv.tv_usec = Timeout;

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

			WriteDataToFile(hFile, (char*)&packetSize, sizeof(packetSize));

			// Receive data
			while (true)
			{
				char* currentAddress = (char*)ReceiveBuffer;

				do {
					fd_set fd;
					FD_ZERO(&fd);
					FD_SET(sock, &fd);

					timeval tv;
					tv.tv_sec = 0;
					tv.tv_usec = Timeout;

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

				WriteDataToFile(hFile, currentAddress, iResult);

				currentSize -= iResult;

				if (currentSize == 0)
					break;

				currentAddress += iResult;
			}
			fileNum++;
		}

	close_connection:
		WSASendDisconnect(sock, NULL);
		closesocket(sock);
		WSACleanup();

		CloseDataFile(hFile);
	}
	catch (exception e)
	{
		WSASendDisconnect(sock, NULL);
		closesocket(sock);
		WSACleanup();
		CloseDataFile(hFile);

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

	fd_set fd;
	FD_ZERO(&fd);
	FD_SET(sock, &fd);

	try {

		iResult = connect(sock, (SOCKADDR*)&boardSockAddr, sizeof(boardSockAddr));
		if (iResult == SOCKET_ERROR)
		{
			throw exception("Connection error");
		}

		// Wait for data or stop flag

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
				FD_ZERO(&fd);
				FD_SET(sock, &fd);

				iResult = select(0, &fd, NULL, NULL, &tv);

				if (stop)
					goto close_connection;

			} while (iResult == 0);

			if (iResult == SOCKET_ERROR)
			{
				string msg = "Read packet size error " + to_string(GetLastError());
				throw exception(msg.c_str());
			}

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
					FD_ZERO(&fd);
					FD_SET(sock, &fd);

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
		WSASendDisconnect(sock, NULL);
		closesocket(sock);
		WSACleanup();

	}
	catch (exception e)
	{
		WSASendDisconnect(sock, NULL);
		closesocket(sock);
		WSACleanup();

		throw;
	}
}

//
// Parse data file, written ReceiveRealTimeData(). 
// Parse each frame and write it in separate file.
//
void ParseDataFile(string fileName, string FilesPath, double &progress, bool& stop)
{
	HANDLE hDataFile = INVALID_HANDLE_VALUE, hWriteFile = INVALID_HANDLE_VALUE;
	int frameNum = 0;
	uint32_t frameSize;
	BOOL b;
	DWORD numberOfBytesRead;
	string writeFileName;
	size_t writeSize;
	LARGE_INTEGER dataFileSize;
	size_t currentSize;

	static char Buffer[1024 * 1024];

	currentSize = 0;

	hDataFile = CreateFileA(
		fileName.c_str(),
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hDataFile == INVALID_HANDLE_VALUE)
	{
		string msg = "Open data file " + fileName + " error: " + to_string(GetLastError());
		throw exception(msg.c_str());
	}

	try {

		if (!GetFileSizeEx(hDataFile, &dataFileSize))
		{
			CloseHandle(hDataFile);
			throw exception("Can't get binary file size");
		}

		while (true)
		{
			// Read frame size
			b = ReadFile(
				hDataFile,
				&frameSize,
				sizeof(frameSize),
				&numberOfBytesRead,
				NULL
			);

			if (!b)
			{
				string msg = "Read frame " + to_string(frameNum) + " size error. Error code: " + to_string(GetLastError());
				throw exception(msg.c_str());
			}

			// End of file 
			if (numberOfBytesRead == 0)
			{
				break;
			}

			if (numberOfBytesRead != sizeof(frameSize))
			{
				string msg = "Read frame " + to_string(frameNum) + " size error. Read bytes number not equal number of bytes to read";
				throw exception(msg.c_str());
			}

			// Create file to frame
			writeFileName = FilesPath + "/" + to_string(frameNum) + ".dat";

			hWriteFile = CreateFileA(
				writeFileName.c_str(),
				GENERIC_WRITE,
				0,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			);

			if (hWriteFile == INVALID_HANDLE_VALUE)
			{
				string msg = "Create file for frame " + to_string(frameNum) + " error. Error code: " + to_string(GetLastError());
				throw exception(msg.c_str());
			}

			// Read frame data and write it to file
			do {
				if (stop)
					goto exit;

				writeSize = frameSize > sizeof(Buffer) ? sizeof(Buffer) : frameSize;

				b = ReadFile(
					hDataFile,
					Buffer,
					writeSize,
					&numberOfBytesRead,
					NULL
				);

				if (!b)
				{
					string msg = "Read frame " + to_string(frameNum) + " data error. Error code: " + to_string(GetLastError());
					throw exception(msg.c_str());
				}

				if (numberOfBytesRead != writeSize)
				{
					// We are reach end of file.
					b = WriteFile(
						hWriteFile,
						Buffer,
						numberOfBytesRead,
						NULL,
						NULL
					);

					goto exit;
				}

				b = WriteFile(
					hWriteFile,
					Buffer,
					writeSize,
					&numberOfBytesRead,
					NULL
				);

				if (!b)
				{
					string msg = "Write frame " + to_string(frameSize) + "data error. Error code: " + to_string(GetLastError());
					throw exception(msg.c_str());
				}

				if (numberOfBytesRead != writeSize)
				{
					string msg = "Write frame " + to_string(frameNum) + " data error. Write bytes number not equal number of bytes to write";
					throw exception(msg.c_str());
				}

				currentSize += writeSize;
				frameSize -= writeSize;
				progress = (double)currentSize / dataFileSize.QuadPart;

			} while (frameSize > 0);

			CloseHandle(hWriteFile);
			hWriteFile = INVALID_HANDLE_VALUE;

			frameNum++;
		}

		CloseHandle(hDataFile);
		hDataFile = INVALID_HANDLE_VALUE;
	}
	catch (exception e)
	{
		if(hDataFile != INVALID_HANDLE_VALUE)
			CloseHandle(hDataFile);

		if(hWriteFile != INVALID_HANDLE_VALUE)
			CloseHandle(hWriteFile);

		throw;
	}

exit:
	if (hDataFile != INVALID_HANDLE_VALUE)
		CloseHandle(hDataFile);

	if (hWriteFile != INVALID_HANDLE_VALUE)
		CloseHandle(hWriteFile);
}
