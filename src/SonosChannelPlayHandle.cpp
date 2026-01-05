#include "SonosChannel.h"
#include "Arduino.h"

SonosChannelPlayHandle::SonosChannelPlayHandle(SonosChannel &channel, String &uri, const char *title, const char *imageUrl, bool isPlaylist, bool isFolder, unsigned long stopCounter)
    : _channel(channel), _uri(uri), _title(title), _imageUrl(imageUrl), _isPlaylist(isPlaylist), _isFolder(isFolder), _stopCounnter(stopCounter)
{
}
bool SonosChannelPlayHandle::isPlaying()
{
    return _channel.isPlaying(this);
}
