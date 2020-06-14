#pragma once

void ReceiveRealTimeData(const char* ip, const short port, bool& stop, int& fileNum);
void ReceiveNoRealTimeData(const char* ip, const short port, size_t size, bool& stop, size_t& actualSize);