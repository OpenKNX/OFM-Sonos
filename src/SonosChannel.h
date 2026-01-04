#pragma once
#include "OpenKNX.h"
#include "SonosApi.h"
#include "WiFi.h"
#include <memory>

class SonosModule;

class SonosChannedPlayHandle
{
friend class SonosChannel;
    private:
        unsigned long _startTime = millis();
        String _uri;
        bool _isPlaylist;
        bool _isFolder;
        unsigned long _stopCounnter;
        unsigned int _playAndTrackChangeCounter = 0;
        bool _playing = true;
    public:
    SonosChannedPlayHandle(String& uri, bool isPlaylist, bool isFolder, unsigned long stopCounter)
        : _uri(uri), _isPlaylist(isPlaylist), _isFolder(isFolder), _stopCounnter(stopCounter)
    {
    }
};

class SonosChannel : public OpenKNX::Channel, protected SonosApiNotificationHandler
{
    private:
        SonosModule& _sonosModule;
        SonosSpeaker* _sonosSpeaker;
        String _name;
        bool _singleControl;
        bool _groupControl;
        SonosTrackInfo _lastTrackInfo;
        SonosApiPlayState _lastPlayState;
        unsigned long _stopCounter = 0;
        unsigned int _playAndTrackChangeCounter = 1;
        void notificationVolumeChanged(SonosSpeaker* speaker, uint8_t volume) override;
        void notificationMuteChanged(SonosSpeaker* speaker, boolean mute) override;
        void notificationGroupVolumeChanged(SonosSpeaker* speaker, uint8_t volume) override;
        void notificationGroupMuteChanged(SonosSpeaker* speaker, boolean mute) override;
        void notificationPlayStateChanged(SonosSpeaker* speaker, SonosApiPlayState playState) override;
        void notificationGroupCoordinatorChanged(SonosSpeaker* speaker) override;
        void notificationTrackChanged(SonosSpeaker* speaker, SonosTrackInfo& trackInfo) override;
        void joinChannel(uint8_t channelNumber);
        void joinNextPlayingGroup();
        bool delegateCoordination(bool rejoinGroup);     
#if ARDUINO_ARCH_ESP32 
        void playNotification(byte notificationNumber);
#endif
    protected:
        void loop1() override;
        void processInputKo(GroupObject &ko) override;
    public:
        SonosChannel(SonosModule& sonosModule, uint8_t _channelIndex /* this parameter is used in macros, do not rename */, SonosApi& sonosApi);
        const IPAddress speakerIP();
        const std::string name() override;
        const std::string logPrefix() override;
        bool processCommand(const std::string cmd, bool diagnoseKo);     
        std::shared_ptr<SonosChannedPlayHandle> start(const char* uri, const char* title, const char* imageUrl, const char* fileUrlPrefix, bool startPlaying);
        void joinToGroupCoordinator(SonosChannel* coordinatorChannel);
        void play(bool play);
        void pause();   
        void shuffle(bool shuffle);
        void setVolumeRelative(int8_t relativeVolume);
        void setGroupVolumeRelative(int8_t relativeVolume);
        void togglePause();
        void nextTrack();
        void previousTrack();
        bool isPlaying(std::shared_ptr<SonosChannedPlayHandle> playHandler);
       
};