#pragma once
#include "OpenKNX.h"

class ConsoleWriterStream : public Stream
{
    bool _error = false;
    char _buffer[OPENKNX_MAX_LOG_MESSAGE_LENGTH];
    int _bufferIndex = 0;
    std::string _logPrefix = "";
public:
    ConsoleWriterStream(const char* logPrefix, bool error);
    size_t write(uint8_t c) override;
    void flush() override;
    int available() override;
    int read() override;
    int peek() override;
    const std::string& logPrefix();

};