#include "SonosChannel.h"
#include "Arduino.h"

SonosChannelPlayHandle::SonosChannelPlayHandle(SonosChannel &channel, String &uri, const char *title, const char *imageUrl, bool isPlaylist, bool isFolder, unsigned long stopCounter, bool startPlaying)
    : _startTime(startPlaying ? max(1UL, millis()) : 0), _channel(channel), _uri(uri), _title(title), _imageUrl(imageUrl), _isPlaylist(isPlaylist), _isFolder(isFolder), _stopCounnter(stopCounter)
{
}
TagPlayState SonosChannelPlayHandle::isPlaying()
{
    return _channel.isPlaying(this);
}
