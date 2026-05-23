#pragma once
#include "OpenKNX.h"
#include "SonosApi.h"
#include "WiFi.h"
#include <memory>
#include "SonosChannelPlayHandle.h"
class SonosModule;

class SonosChannel : public OpenKNX::Channel, protected SonosApiNotificationHandler
{
    private:
        bool _locked = false;
        bool _online = false;
        bool _enabled = false;
        bool _forcePing = false;
        unsigned long _lastPingTime = 0;
        SonosModule& _sonosModule;
        SonosSpeaker* _sonosSpeaker = nullptr;
        String _name;
        bool _singleControl;
        bool _groupControl;
        SonosTrackInfo _lastTrackInfo;
        SonosApiPlayState _lastPlayState;
        unsigned long _stopCounter = 0;
        unsigned long _waitForSetNotifiationHandler = 0;
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
        void online(bool online); 
        void updateLockState(bool initialize = false);
#if ARDUINO_ARCH_ESP32 
        void playNotification(byte notificationNumber);
#endif
    protected:
        void loop() override;
        void processInputKo(GroupObject &ko) override;
    public:
        SonosChannel(SonosModule& sonosModule, uint8_t _channelIndex /* this parameter is used in macros, do not rename */, SonosApi& sonosApi);
        void setup() override;
        const IPAddress speakerIP();
        const std::string name() override;
        const std::string logPrefix() override;
        bool processCommand(const std::string cmd, bool diagnoseKo);     
        std::shared_ptr<SonosChannelPlayHandle> start(const char* uri, const char* title, const char* imageUrl, const char* fileUrlPrefix, bool startPlaying);
        void start(const std::shared_ptr<SonosChannelPlayHandle>& playHandler);
        void joinToGroupCoordinatorOf(SonosChannel* channel);
        void unjoin();
        void play(bool play = true);
        void pause();   
        void shuffle(bool shuffle);
        void setVolumeRelative(int8_t relativeVolume);
        void setVolume(uint8_t volume);
        void setGroupVolumeRelative(int8_t relativeVolume);
        void setGroupVolume(uint8_t volume);
        uint8_t getGroupVolume();
        void togglePause();
        void nextTrack();
        void previousTrack();
        SonosApiPlayState getPlayState();
        TagPlayState isPlaying(SonosChannelPlayHandle* playHandler);
        uint8_t getChannelIndex();
        bool isOnline() const;
   
};