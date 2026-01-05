#pragma once
#include "Arduino.h"
class SonosChannel;

class SonosChannelPlayHandle
{
friend class SonosChannel;
    private:
        unsigned long _startTime = millis();
        String _uri;
        String _title;
        String _imageUrl;
        bool _isPlaylist;
        bool _isFolder;
        unsigned long _stopCounnter;
        unsigned int _playAndTrackChangeCounter = 0;
        bool _playing = true;
        SonosChannel& _channel;
    public:
    SonosChannelPlayHandle(SonosChannel& channel, String& uri, const char* title, const char* imageUrl, bool isPlaylist, bool isFolder, unsigned long stopCounter);
    bool isPlaying();

};