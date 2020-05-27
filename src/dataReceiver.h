#pragma once

#include <string>

void ReceiveRealTimeData(const char* ip, const short port, bool& stop, int& fileNum);
void ReceiveNoRealTimeData(const char* ip, const short port, size_t size, bool& stop, size_t& actualSize);

//
// Parse data file, written ReceiveRealTimeData(). 
// Parse each frame and write it in separate file.
//
void ParseDataFile(std::string fileName, std::string FilesPath, double& progress, bool& stop);
