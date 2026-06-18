#include "ConsoleWriterStream.h"
#include "OpenKNX.h"

ConsoleWriterStream::ConsoleWriterStream(const char* logPrefix, bool error) : 
    _logPrefix(logPrefix), 
    _error(error) 
{

}

const std::string& ConsoleWriterStream::logPrefix()
{
    return _logPrefix;
}

size_t ConsoleWriterStream::write(uint8_t c)
{
    if (c == '\n' || c == '\r' || c == 0)
    {
        flush();
    }
    else
    {
        _buffer[_bufferIndex++] = c;
        if (_bufferIndex >= OPENKNX_MAX_LOG_MESSAGE_LENGTH - 1)
        {
            flush();
        }
    }
    return 1;
}
void ConsoleWriterStream::flush()
{
    if (_bufferIndex > 0)
    {
        _buffer[_bufferIndex] = 0;
        _bufferIndex = 0;
        if (strlen(_buffer) > 0)
        {
            if (_error)
                logErrorP(_buffer);
            else
                logDebugP(_buffer);
        }
    }
}
int ConsoleWriterStream::available() { return 0; }
int ConsoleWriterStream::read() { return -1; }
int ConsoleWriterStream::peek() { return -1; }
