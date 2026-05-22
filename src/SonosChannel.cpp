#include "SonosChannel.h"
#include "SonosModule.h"

void replaceAll(std::string& str,
                const std::string& from,
                const std::string& to)
{
    if (from.empty()) return; 

    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length(); // wichtig!
    }
}

SonosChannel::SonosChannel(SonosModule& sonosModule, uint8_t _channelIndex /* this parameter is used in macros, do not rename */, SonosApi& sonosApi)
    : _sonosModule(sonosModule), _name()
{
    this->_channelIndex = _channelIndex;
    // <Enumeration Text="Inaktiv"                          Value="0" Id="%ENID%" />
    // <Enumeration Text="Einzelsteuerung"                  Value="2" Id="%ENID%" />
    // <Enumeration Text="Gruppensteuerung"                 Value="3" Id="%ENID%" />
    // <Enumeration Text="Einzel- und Gruppensteurung"      Value="4" Id="%ENID%" />
    // <Enumeration Text="Nur Gruppenteilnehmer"            Value="5" Id="%ENID%" />            
    auto usage = ParamSON_CHSonosChannelUsage;
    _singleControl = usage == PT_SONSonosChannelUsage::SingleControl || usage == PT_SONSonosChannelUsage::SingleAndGroupControl;
    _groupControl = usage == PT_SONSonosChannelUsage::GroupControl || usage == PT_SONSonosChannelUsage::SingleAndGroupControl;
    auto parameterIP = (uint32_t)ParamSON_CHSonosIPAddress;
    uint32_t arduinoIP = ((parameterIP & 0xFF000000) >> 24) | ((parameterIP & 0x00FF0000) >> 8) | ((parameterIP & 0x0000FF00) << 8) | ((parameterIP & 0x000000FF) << 24);
    IPAddress speakerIP = IPAddress(arduinoIP);
    _sonosSpeaker = sonosApi.addSpeaker(speakerIP);
    _name = speakerIP.toString();
}

void SonosChannel::setup()
{
    if (ParamSON_CHChannelEnableKo)
    {
        if (KoSON_CHChannelEnabled.initialized())
        {
            lockChannel(!KoSON_CHChannelEnabled.value(DPT_Switch));
        }
        else
        {
            // if KO is not initialized, initialize it with current state
            lockChannel(true); // lock channel until state is read
            KoSON_CHChannelEnabled.requestObjectRead();
        }
    }
    else
    {
        lockChannel(false);
    }
}


const IPAddress SonosChannel::speakerIP()
{
    return _sonosSpeaker->getSpeakerIP();
}

const std::string SonosChannel::name()
{
    return std::string(_name.c_str());
}
const std::string SonosChannel::logPrefix()
{
    return "Sonos.Channel";
}

void SonosChannel::lockChannel(bool lock)
{
    if (_locked == lock)
        return;
    _locked = lock;
    if (_locked)
    {
        KoSON_CHPlayFeedback.valueCompare(false, DPT_Start);   
    }
#ifndef SONOS_DISABLE_CALLBACK
    _sonosSpeaker->setCallback(_locked ? nullptr : this);
#endif
}

void SonosChannel::loop()
{
    if (_locked)
        return;
    if (_sonosSpeaker != nullptr)
        _sonosSpeaker->loop();
}

void SonosChannel::notificationVolumeChanged(SonosSpeaker* speaker, uint8_t volume)
{
    if (_singleControl && volume != (uint8_t)KoSON_CHVolumeState.value(DPT_Scaling))
        KoSON_CHVolumeState.value(volume, DPT_Scaling);
}

void SonosChannel::notificationMuteChanged(SonosSpeaker* speaker, boolean mute)
{
    if (_singleControl && mute != (boolean)KoSON_CHMuteState.value(DPT_Switch))
        KoSON_CHMuteState.value(mute, DPT_Switch);
}

void SonosChannel::notificationGroupVolumeChanged(SonosSpeaker* speaker, uint8_t volume)
{
    if (_groupControl && volume != (uint8_t)KoSON_CHGroupVolumeState.value(DPT_Scaling))
        KoSON_CHGroupVolumeState.value(volume, DPT_Scaling);
    if (_sonosSpeaker == speaker)
    {
        // this channel is group coordinator, notify all participants
        for (int channelIndex = 0; channelIndex < _sonosModule.getNumberOfChannels(); channelIndex++)
        {
            if (channelIndex == _channelIndex)
                continue;
            auto channel = (SonosChannel*)_sonosModule.getChannel(channelIndex);
            if (channel != nullptr && channel->_sonosSpeaker->findGroupCoordinator(true) == speaker)
            {
                channel->notificationGroupVolumeChanged(speaker, volume);
            }
        }
    }
}

void SonosChannel::notificationGroupMuteChanged(SonosSpeaker* speaker, boolean mute)
{
    if (_groupControl && mute != (boolean)KoSON_CHGroupMuteState.value(DPT_Switch))
        KoSON_CHGroupMuteState.value(mute, DPT_Switch);
    if (_sonosSpeaker == speaker)
    {
        // this channel is group coordinator, notify all participants
        for (int channelIndex = 0; channelIndex < _sonosModule.getNumberOfChannels(); channelIndex++)
        {
            if (channelIndex == _channelIndex)
                continue;
            auto channel = (SonosChannel*)_sonosModule.getChannel(channelIndex);
            if (channel != nullptr && channel->_sonosSpeaker->findGroupCoordinator(true) == speaker)
            {
                channel->notificationGroupMuteChanged(speaker, mute);
            }
        }
    }
}

void SonosChannel::notificationPlayStateChanged(SonosSpeaker* speaker, SonosApiPlayState playState)
{
    if (playState != _lastPlayState && playState == SonosApiPlayState::Stopped)
    {
        _stopCounter++;
    }
    _lastPlayState = playState;
    _playAndTrackChangeCounter = max((unsigned int) 1, _playAndTrackChangeCounter + 1);
    if (_sonosSpeaker->findGroupCoordinator() == _sonosSpeaker)
    {
        // this channel is group coordinator
        bool playing = playState == SonosApiPlayState::Playing || playState == SonosApiPlayState::Transitioning;
        if (_singleControl && playing != (boolean)KoSON_CHPlayFeedback.value(DPT_Start))
            KoSON_CHPlayFeedback.value(playing, DPT_Start);

        // notify all participants
        for (int channelIndex = 0; channelIndex < _sonosModule.getNumberOfChannels(); channelIndex++)
        {
            if (channelIndex == _channelIndex)
                continue;
            auto channel = (SonosChannel*)_sonosModule.getChannel(channelIndex);
            if (channel != nullptr && channel->_sonosSpeaker->findGroupCoordinator(true) == speaker)
            {
                channel->notificationPlayStateChanged(speaker, playState);
            }
        }
    }
    else if (speaker != _sonosSpeaker)
    {
        // called from master, take over play state
        bool playing = playState == SonosApiPlayState::Playing || playState == SonosApiPlayState::Transitioning;
        if (_singleControl && playing != (boolean)KoSON_CHPlayFeedback.value(DPT_Start))
            KoSON_CHPlayFeedback.value(playing, DPT_Start);
    }
}

void SonosChannel::notificationGroupCoordinatorChanged(SonosSpeaker* speaker)
{
    auto groupCoordinator = _sonosSpeaker->findGroupCoordinator();
    if (groupCoordinator == nullptr)
        return;
    auto playState = groupCoordinator->getPlayState();
    bool playing = playState == SonosApiPlayState::Playing || playState == SonosApiPlayState::Transitioning;
    if (_singleControl && playing != (boolean)KoSON_CHPlayFeedback.value(DPT_Start))
        KoSON_CHPlayFeedback.value(playing, DPT_Start);
    auto groupVolume = groupCoordinator->getGroupVolume();
    if (_groupControl && groupVolume != (uint8_t)KoSON_CHGroupVolumeState.value(DPT_Scaling))
        KoSON_CHGroupVolumeState.value(groupVolume, DPT_Scaling);
    auto groupMute = groupCoordinator->getGroupMute();
    if (_singleControl && groupMute != (boolean)KoSON_CHGroupMuteState.value(DPT_Switch))
        KoSON_CHGroupMuteState.value(groupMute, DPT_Switch);
}


TagPlayState SonosChannel::isPlaying(SonosChannelPlayHandle* playHandler)
{
    if (_locked)
        return TagPlayState::Stopped;
    if (playHandler == nullptr)
        return TagPlayState::Stopped;
    
    SonosChannelPlayHandle& handle = *playHandler;
    auto currentPlayAndTrackChangeCounter = _playAndTrackChangeCounter;
    if (playHandler->_playAndTrackChangeCounter != currentPlayAndTrackChangeCounter)
    {
        // Update state
        bool playing = true;
        handle._playAndTrackChangeCounter = currentPlayAndTrackChangeCounter;
        if (_lastPlayState != SonosApiPlayState::Playing && _lastPlayState != SonosApiPlayState::Transitioning)
        {
            logDebugP("Not playing, last play state %d", (int) _lastPlayState);
            playing = false;
        }
        if (playHandler->_isPlaylist)
        {
            bool stopCounterChanged = _stopCounter != playHandler->_stopCounnter;
            if (stopCounterChanged)
            {
                logDebugP("Stop counter changed from %lu to %lu", playHandler->_stopCounnter, _stopCounter);
                playing = false;
            }
            else
            {
                playing = handle._playing;
                if (!playing)
                    logDebugP("Playlist already stopped");
            }
        }
        else
        {
        
            String uri = _lastTrackInfo.uri;
            String uriHandle = playHandler->_uri.c_str();
            uri.replace("https:", "http:");
            uri.replace("&amp;", "%26");
            uriHandle.replace("https:", "http:");
            uriHandle.replace("&amp;", "%26");
            if (uriHandle.startsWith("x-rincon-mp3radio://"))
                 uri.replace("aac://", "x-rincon-mp3radio://");
            logDebugP("Checking playing state for card URL '%s'", uriHandle.c_str());
            if (playHandler->_isFolder)
            {
                if (!uri.startsWith(uriHandle))
                {
                    logInfoP("URL  '%s' does not start with", uriHandle.c_str());
                    logInfoP("Card '%s'", uri.c_str());
                    playing = false;
                }
            }
            else if (uri != uriHandle)
            {
                logInfoP("URL '%s' does not match card URL '%s'", uri.c_str(), uriHandle.c_str());
                playing = false;
            }
        }
        if (playing)
        {
            logDebugP("Is playing");
            handle._startTime = 0; // reset start time
        }
        handle._playing = playing;
    }
    if (handle._startTime != 0)
    {
        if (millis() - handle._startTime > 5000)
        {
            logDebugP("Start wait time finished");
            handle._startTime = 0;
            handle._timeout = true;
        }
        else
        {
             return TagPlayState::WaitForResponse;
        }
    }
    return handle._playing ? TagPlayState::Playing : TagPlayState::Stopped;
   
}

void SonosChannel::notificationTrackChanged(SonosSpeaker* speaker, SonosTrackInfo& trackInfo)
{
    _lastTrackInfo = trackInfo;
    _playAndTrackChangeCounter = max((unsigned int) 1, _playAndTrackChangeCounter + 1);
  
    uint8_t sourceNumber = 0;
    if (trackInfo.uri.length() != 0)
    {
        for (uint8_t _channelIndex = 0; _channelIndex < SONSRC_ChannelCount && _channelIndex < SONSRC_VisibleChannels && sourceNumber == 0; _channelIndex++)
        {
            auto sourceType = ParamSONSRC_CHSourceType;
            switch (sourceType)
            {
                case PT_SONSourceType::RadioStream:
                {
                    if (trackInfo.uri.startsWith(SonosApi::DefaultSchemaInternetRadio))
                    {
                        if (strcmp(trackInfo.uri.c_str() + strlen(SonosApi::DefaultSchemaInternetRadio), ParamSONSRC_CHSourceUriStr.c_str()) == 0)
                            sourceNumber = _channelIndex + 1;
                    }
                    break;
                }
                case PT_SONSourceType::Http: 
                {
                    if (strcmp(trackInfo.uri.c_str(), ParamSONSRC_CHSourceUriStr.c_str()) == 0)
                        sourceNumber = _channelIndex + 1;
                    break;
                }
                case PT_SONSourceType::MusicLibraryFile: 
                {
                    if (trackInfo.uri.startsWith(SonosApi::SchemaMusicLibraryFile))
                    {
                        String uri = ParamSONSRC_CHSourceUriStr.c_str();
                        if (strcmp(trackInfo.uri.c_str() + strlen(SonosApi::SchemaMusicLibraryFile), uri.c_str()) == 0)
                            sourceNumber = _channelIndex + 1;
                    }
                    break;
                }
                case PT_SONSourceType::MusicLibraryDirectory: 
                {
                    if (trackInfo.uri.startsWith(SonosApi::SchemaMusicLibraryFile))
                    {
                        String uri = ParamSONSRC_CHSourceUriStr.c_str();
                        uri.replace(" ", "%20");
                        if (!uri.endsWith("/"))
                            uri += "/";
                        if (strncmp(trackInfo.uri.c_str() + strlen(SonosApi::SchemaMusicLibraryFile), uri.c_str(), uri.length()) == 0)
                            sourceNumber = _channelIndex + 1;
                    }
                    break;
                }
                case PT_SONSourceType::LineIn: 
                {
                    if (trackInfo.uri.startsWith(SonosApi::SchemaLineIn))
                    {
                        auto groupCoordinator = _sonosSpeaker->findGroupCoordinator();
                        if (groupCoordinator != nullptr &&
                            strcmp(trackInfo.uri.c_str() + strlen(SonosApi::SchemaLineIn), groupCoordinator->getUID().c_str()) == 0)
                            sourceNumber = _channelIndex + 1;
                    }
                    break;
                }
                case PT_SONSourceType::TVIn: 
                {
                    if (trackInfo.uri.startsWith(SonosApi::SchemaTVIn))
                    {
                        auto groupCoordinator = _sonosSpeaker->findGroupCoordinator();
                        if (groupCoordinator != nullptr)
                        {
                            auto url = groupCoordinator->getUID() + SonosApi::UrlPostfixTVIn;
                            if (strcmp(trackInfo.uri.c_str() + strlen(SonosApi::SchemaTVIn), url.c_str()) == 0)
                                sourceNumber = _channelIndex + 1;
                        }
                    }
                    break;
                }
                case PT_SONSourceType::SonosPlaylist: 
                {
                    // Can not be detected
                }
                case PT_SONSourceType::SonosUri: 
                {
                    if (trackInfo.uri == ParamSONSRC_CHSourceUriStr.c_str())
                        sourceNumber = _channelIndex + 1;
                }
            }
        }
    }
    Serial.print("Source State: ");
    Serial.println(sourceNumber);
    if ((_singleControl || _groupControl) && (uint8_t)KoSON_CHSourceState.value(DPT_Value_1_Ucount) != sourceNumber)
    {
        KoSON_CHSourceState.value(sourceNumber, DPT_Value_1_Ucount);
    }
}

void SonosChannel::processInputKo(GroupObject& ko)
{
    auto index = SON_KoCalcIndex(ko.asap());
    if (index == SON_KoCHChannelEnabled)
    {
        boolean enabled = ko.value(DPT_Switch);
        logDebugP("Set channel enabled %d", enabled);
        if (enabled)
            lockChannel(false);
        else
            lockChannel(true);
        return;
    }
    if (_locked)
    {
        logDebugP("Channel is locked, ignoring command");
        return;
    }
    switch (index)
    {
        case SON_KoCHVolume:
        {
            uint8_t volume = ko.value(DPT_Scaling);
            logDebugP("Set volume %d", volume);
            _sonosSpeaker->setVolume(volume);
            break;
        }
        case SON_KoCHVolumeRelativ:
        {
            bool increase = ko.value(DPT_Step);
            auto volume = increase ? (int8_t)ParamSON_CHRelativVolumeStep : -(int8_t)ParamSON_CHRelativVolumeStep;
            logDebugP("Set volume relative %d", volume);
            _sonosSpeaker->setVolumeRelative(volume);
            break;
        }
        case SON_KoCHMute:
        {
            boolean mute = ko.value(DPT_Switch);
            logDebugP("Set mute %d", mute);
            _sonosSpeaker->setMute(mute);
            break;
        }
        case SON_KoCHGroupVolume:
        {
            uint8_t volume = ko.value(DPT_Scaling);
            logDebugP("Set group volume %d", volume);
            _sonosSpeaker->setGroupVolume(volume);
            break;
        }
        case SON_KoCHGroupVolumeRelativ:
        {
            bool increase = ko.value(DPT_Step);
            auto volume = increase ? (int8_t)ParamSON_CHGroupRelativVolumeStep : -(int8_t)ParamSON_CHGroupRelativVolumeStep;
            logDebugP("Set volume relative %d", volume);
            auto groupCoordinator = _sonosSpeaker->findGroupCoordinator();
            if (groupCoordinator != nullptr)
                groupCoordinator->setGroupVolumeRelative(volume);
            break;
        }
        case SON_KoCHGroupMute:
        {
            boolean mute = ko.value(DPT_Switch);
            logDebugP("Set group mute %d", mute);
            auto groupCoordinator = _sonosSpeaker->findGroupCoordinator();
            if (groupCoordinator != nullptr)
                groupCoordinator->setGroupMute(mute);
            break;
        }
        case SON_KoCHPlay:
        {
            play(ko.value(DPT_Switch));
            break;
        }
        case SON_KoCHPreviousNext:
        {
            boolean next = ko.value(DPT_UpDown);
            if (next)
            {
                logDebugP("Set Next");
                _sonosSpeaker->next();
            }
            else
            {
                logDebugP("Set Previous");
                _sonosSpeaker->previous();
            }
            break;
        }
        case SON_KoCHStop:
        {
            boolean trigger = ko.value(DPT_Trigger);
            if (trigger)
            {
                logDebugP("Set Stop");
                _sonosSpeaker->stop();
            }
            break;
        }
        case SON_KoCHJoinNextActiveGroup:
        {
            boolean trigger = ko.value(DPT_Trigger);
            if (trigger)
            {
                logDebugP("Join next playing group");
                joinNextPlayingGroup();
            }
            break;
        }
        case SON_KoCHJoinChannelNumber:
        {
            uint8_t channelNumber = ko.value(DPT_Value_1_Ucount);
            logDebugP("Join channel %d", channelNumber);
            joinChannel(channelNumber);
            break;
        }
        case SON_KoCHSourceNumber:
        {
            uint8_t sourceNumber = ko.value(DPT_Value_1_Ucount);
            if (sourceNumber < 1 || sourceNumber > SONSRC_ChannelCount || sourceNumber > ParamSONSRC_VisibleChannels)
                return;
            uint8_t _channelIndex = sourceNumber - 1;
            auto sourceType = ParamSONSRC_CHSourceType;
            auto groupCoordinator = _sonosSpeaker->findGroupCoordinator();
            if (groupCoordinator == nullptr)
                return;
            switch (sourceType)
            {
                case PT_SONSourceType::RadioStream:
                {
                    groupCoordinator->playInternetRadio(ParamSONSRC_CHSourceUriStr.c_str(), ParamSONSRC_CHSourceTitleStr.c_str(), ParamSONSRC_CHSourceUriImageStr.c_str());
                    break;
                }
                case PT_SONSourceType::Http:
                {
                    groupCoordinator->playFromHttp(ParamSONSRC_CHSourceUriStr.c_str());
                    break;
                }
                case PT_SONSourceType::MusicLibraryFile:
                {
                    groupCoordinator->playMusicLibraryFile(ParamSONSRC_CHSourceUriStr.c_str());
                    break;
                }
                case PT_SONSourceType::MusicLibraryDirectory:
                {
                    groupCoordinator->setShuffle(ParamSONSRC_CHRandom);
                    groupCoordinator->playMusicLibraryDirectory(ParamSONSRC_CHSourceUriStr.c_str());
                    break;
                }
                case PT_SONSourceType::LineIn:
                {
                    if (groupCoordinator != _sonosSpeaker)
                    {
                        groupCoordinator->delegateGroupCoordinationTo(_sonosSpeaker, false);
                    }
                    _sonosSpeaker->playLineIn();
                    break;
                }
                case PT_SONSourceType::TVIn:
                {
                    if (groupCoordinator != _sonosSpeaker)
                    {
                        groupCoordinator->delegateGroupCoordinationTo(_sonosSpeaker, false);
                    }
                    _sonosSpeaker->playTVIn();
                    break;
                }
                case PT_SONSourceType::SonosPlaylist: 
                {
                    groupCoordinator->setShuffle(ParamSONSRC_CHRandom);
                    groupCoordinator->playSonosPlaylist(ParamSONSRC_CHSourceUriStr.c_str());
                    break;
                }
                case PT_SONSourceType::SonosUri: 
                {
                    if (!ParamSONSRC_CHSourceTitleStr.empty() || !ParamSONSRC_CHSourceUriImageStr.empty())
                        groupCoordinator->playInternetRadio(ParamSONSRC_CHSourceUriStr.c_str(), ParamSONSRC_CHSourceTitleStr.c_str(), ParamSONSRC_CHSourceUriImageStr.c_str(), "");
                    else
                    {
                        groupCoordinator->setShuffle(ParamSONSRC_CHRandom);
                        groupCoordinator->setAVTransportURI(nullptr, ParamSONSRC_CHSourceUriStr.c_str());     
                        groupCoordinator->play();
                    }
                    break;
                }
            }
            break;
        }
        case SON_KoCHNotificationStart:
        {
            boolean trigger = ko.value(DPT_Trigger);
            if (trigger)
            {
                byte notificationNumber = ParamSON_CHStandardNotification;
                logDebugP("play notification %d", notificationNumber);      
#if ARDUINO_ARCH_ESP32    
                playNotification(notificationNumber);
#endif
            }
            break;
        }
        case SON_KoCHNotificationNumber:
        {
            byte notificationNumber = ko.value(DPT_Value_1_Ucount);
            logDebugP("play notification %d", notificationNumber);
#if ARDUINO_ARCH_ESP32    
            playNotification(notificationNumber);
#endif
            break;
        }
    }
}

void SonosChannel::play(bool play)
{
    if (_locked)
        return;
    auto groupCoordinator = _sonosSpeaker->findGroupCoordinator();
    if (groupCoordinator == nullptr)
        return;
    if (play)
        groupCoordinator->play();
    else
        groupCoordinator->pause();
}

void SonosChannel::pause()
{
    if (_locked)
        return;
    play(false); 
}

void SonosChannel::shuffle(bool shuffle)
{
    if (_locked)
        return;
    auto groupCoordinator = _sonosSpeaker->findGroupCoordinator();
    if (groupCoordinator == nullptr)
        return;
    groupCoordinator->setShuffle(shuffle);
}

void SonosChannel::start(const std::shared_ptr<SonosChannelPlayHandle>& playHandler)
{
    if (_locked)
        return;
    if (playHandler == nullptr)
        return;
    start(playHandler->_uri.c_str(), playHandler->_title.c_str(), playHandler->_imageUrl.c_str(), "", true);
}

std::shared_ptr<SonosChannelPlayHandle> SonosChannel::start(const char* uri, const char* title, const char* imageUrl, const char* fileUrlPrefix, bool startPlaying)
{
    if (_locked)
        return std::shared_ptr<SonosChannelPlayHandle>();
    static const char hex[] = "0123456789ABCDEF";
   
    if (uri == nullptr || uri[0] == '\0')
        return std::shared_ptr<SonosChannelPlayHandle>();
    String uriStr;
    while (char c = *uri++)
    {
        if (c >= 0x21 && c <= 0x7e)
        {
            uriStr += c;
        }
        else
        {
            uriStr += '%';
            uriStr += hex[c >> 4];
            uriStr += hex[c & 15];
        }
    }
    logDebugP("Start URI '%s'", uriStr.c_str());
    if (uriStr.startsWith("x-file-cifs:"))
    {
        String filePath = uriStr.c_str() + 12;
        String absolutePath = filePath;
        if (!filePath.startsWith("//"))
        {
            // build absolute path, take care of single slash as separator
            absolutePath = fileUrlPrefix;
            if (absolutePath.endsWith("/"))
            {
                if (filePath.startsWith("/"))
                    absolutePath = absolutePath + filePath.substring(1);
                else
                    absolutePath = absolutePath + filePath;
            }
            else
            {
                if (filePath.startsWith("/"))
                    absolutePath = absolutePath + filePath;
                else
                    absolutePath = absolutePath + "/" + filePath;
            }
        }
        if (startPlaying)
            logDebugP("Playing file path '%s'", absolutePath.c_str());
        else
            logDebugP("Use file path '%s'", absolutePath.c_str());
        if (absolutePath.endsWith("/"))
        {
            // end with / -> directory found
            if (startPlaying)
                _sonosSpeaker->playMusicLibraryDirectory(absolutePath.c_str());          
            return std::make_shared<SonosChannelPlayHandle>(*this, "x-file-cifs:" + absolutePath, title, imageUrl, false, true, _stopCounter, startPlaying);
        }
        else
        {
            if (startPlaying)
                _sonosSpeaker->playMusicLibraryFile(absolutePath.c_str());
            return std::make_shared<SonosChannelPlayHandle>(*this, "x-file-cifs:" + absolutePath, title, imageUrl, false, false, _stopCounter, startPlaying);
        }
    }
    else if (uriStr.startsWith("x-playlist:"))
    {
        if (startPlaying)
            _sonosSpeaker->playSonosPlaylist(uriStr.c_str() + 11);
        return std::make_shared<SonosChannelPlayHandle>(*this, uriStr, title, imageUrl, true, false, _stopCounter, startPlaying);
    }
    else if (uriStr.startsWith("x-rincon-mp3radio://"))
    {
        if (title == nullptr || strlen(title) == 0 || title[0] == '\0')
            title = "Radio";
        if (startPlaying)
            _sonosSpeaker->playInternetRadio(uriStr.c_str() + 20, title, imageUrl);
        return std::make_shared<SonosChannelPlayHandle>(*this, uriStr, title, imageUrl, false, false, _stopCounter, startPlaying);
    }
    return std::shared_ptr<SonosChannelPlayHandle>();
    
}

void SonosChannel::joinToGroupCoordinatorOf(SonosChannel* channel)
{
    if (_locked)
        return;
    if (channel == nullptr)
        return;
    auto groupCoordinator = channel->_sonosSpeaker->findGroupCoordinator();
    if (groupCoordinator != nullptr)
    {
        _sonosSpeaker->joinToGroupCoordinator(groupCoordinator);
    }
}

void SonosChannel::joinChannel(uint8_t channelNumber)
{
    if (_locked)
        return;
    if (channelNumber > 0 && channelNumber <= _sonosModule.getNumberOfChannels())
    {
        auto sonosChannel = (SonosChannel*)_sonosModule.getChannel(channelNumber - 1);
        if (sonosChannel != nullptr)
        {
            if (sonosChannel == this)
            {
                channelNumber = 0;
            }
            else
            {
                auto groupCoordinator = sonosChannel->_sonosSpeaker->findGroupCoordinator();
                _sonosSpeaker->joinToGroupCoordinator(groupCoordinator);
            }
        }
    }
    if (channelNumber == 0)
    {
        if (!delegateCoordination(false))
            _sonosSpeaker->unjoin();
    }
}

void SonosChannel::unjoin()
{
    if (_locked)
        return;
    joinChannel(0);
}

bool SonosChannel::processCommand(const std::string cmd, bool diagnoseKo)
{
    if (cmd == "uid")
    {
        Serial.println();
        Serial.println(_sonosSpeaker->getUID().c_str());
        return true;
    }
    if (cmd == "track")
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        auto trackInfo = _sonosSpeaker->getTrackInfo();
        Serial.println(trackInfo.trackNumber);
        Serial.println(trackInfo.duration);
        Serial.println(trackInfo.position);
        Serial.println(trackInfo.uri);
        Serial.println(trackInfo.metadata);
    }
    else if (cmd.rfind("vol ", 0) == 0)
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        int value = atoi(cmd.c_str() + 4);
        if (value < 0 || value > 100)
            Serial.printf("Invalid volume %d\r\n", value);
        else
            setVolume(value);
    }
    else if (cmd.rfind("rvol ", 0) == 0)
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        int value = atoi(cmd.c_str() + 5);
        if (value < -100 || value > 100)
            Serial.printf("Invalid relative volume %d\r\n", value);
        else
            setVolumeRelative(value);
    }
    else if (cmd.rfind("gvol ", 0) == 0)
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        int value = atoi(cmd.c_str() + 5);
        if (value < 0 || value > 100)
            Serial.printf("Invalid volume %d\r\n", value);
        else
            setGroupVolume(value);
    }
    else if (cmd.rfind("rgvol ", 0) == 0)
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        int value = atoi(cmd.c_str() + 6);
        if (value < -100 || value > 100)
            Serial.printf("Invalid relative volume %d\r\n", value);
        else
            setGroupVolumeRelative(value);
    }
    else if (cmd.rfind("treb ", 0) == 0)
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        int value = atoi(cmd.c_str() + 5);
        if (value < -10 || value > 10)
            Serial.printf("Invalid treble %d\r\n", value);
        else
            _sonosSpeaker->setTreble(value);
    }
    else if (cmd == "treb")
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }      
        Serial.println(_sonosSpeaker->getTreble());
        Serial.println(_sonosSpeaker->getTreble());
    }
    else if (cmd.rfind("bass ", 0) == 0)
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        int value = atoi(cmd.c_str() + 5);
        if (value < -10 || value > 10)
            Serial.printf("Invalid bass %d\r\n", value);
        else
            _sonosSpeaker->setBass(value);
    }
    else if (cmd == "bass")
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        Serial.println(_sonosSpeaker->getBass());
    }
    else if (cmd.rfind("mute ", 0) == 0)
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        bool mute = cmd.substr(5) == "1";
        _sonosSpeaker->setMute(mute);
    }
    else if (cmd.rfind("gmute ", 0) == 0)
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        bool mute = cmd.substr(6) == "1";
        _sonosSpeaker->setGroupMute(mute);
    }
    else if (cmd == "vol")
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        Serial.println(_sonosSpeaker->getVolume());
    }
    else if (cmd == "gvol")
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        Serial.println(_sonosSpeaker->getGroupVolume());
    }
    else if (cmd.rfind("ldn ", 0) == 0)
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        bool loudness = cmd.substr(4) == "1";
        _sonosSpeaker->setLoudness(loudness);
    }
    else if (cmd == "ldn")
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        Serial.println(_sonosSpeaker->getLoudness() ? "1" : "0");
    }
    else if (cmd.rfind("led ", 0) == 0)
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        bool on = cmd.substr(4) == "1";
        _sonosSpeaker->setStatusLight(on);
    }
    else if (cmd == "led")
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        Serial.println(_sonosSpeaker->getStatusLight() ? "1" : "0");
    }
    else if (cmd == "mute")
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        Serial.println(_sonosSpeaker->getMute() ? "1" : "0");
    }
    else if (cmd == "gmute")
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        Serial.println(_sonosSpeaker->getGroupMute() ? "1" : "0");
    }
    else if (cmd == "state")
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        Serial.println(_sonosSpeaker->getPlayState());
    }
    else if (cmd == "play")
    {
        Serial.println();
        if (_locked)        
        {
            Serial.println("Channel is locked");
            return true;
        }
        _sonosSpeaker->play();
    }
    else if (cmd == "pause")
    {
        Serial.println();        
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        _sonosSpeaker->pause();
    }
    else if (cmd == "next")
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        _sonosSpeaker->next();
    }
    else if (cmd == "prev")
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        _sonosSpeaker->previous();
    }
    else if (cmd == "stop")
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        _sonosSpeaker->stop();
    }
    else if (cmd == "findc")
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        auto groupCoordinator = _sonosSpeaker->findGroupCoordinator();
        if (groupCoordinator != nullptr)
            Serial.println(groupCoordinator->getSpeakerIP().toString());
        else
            Serial.println("Not found");
    }
    else if (cmd == "findnpc")
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        auto groupCoordinator = _sonosSpeaker->findNextPlayingGroupCoordinator();
        if (groupCoordinator != nullptr)
            Serial.println(groupCoordinator->getSpeakerIP().toString());
        else
            Serial.println("Not found");
    }
    else if (cmd == "joinnext")
    {
        Serial.println();
        if (_locked)
        {
            Serial.println("Channel is locked");
            return true;
        }
        joinNextPlayingGroup();
    }
    else if (cmd.rfind("dele ", 0) == 0)
    {
        Serial.println();
        if (_locked)        
        {
            Serial.println("Channel is locked");
            return true;
        }
        auto targetChannel = atoi(cmd.substr(5).c_str());
        if (targetChannel < 1 || targetChannel > _sonosModule.getNumberOfChannels())
        {
            Serial.print("Invalid channel ");
            Serial.println(targetChannel);
            return true;
        }
        if (targetChannel - 1 == _channelIndex)
        {
            Serial.println("Target channel must be a differnt channel");
            return true;
        }
        auto channel = (SonosChannel*)_sonosModule.getChannel(targetChannel - 1);
        if (channel == nullptr)
        {
            Serial.print("Channel ");
            Serial.print(targetChannel);
            Serial.println("is deactivated");
            return true;
        }
        _sonosSpeaker->delegateGroupCoordinationTo(channel->_sonosSpeaker, true);
    }
    else if (cmd.rfind("join ", 0) == 0)
    {
        Serial.println();
        if (_locked)        
        {
            Serial.println("Channel is locked");
            return true;
        }
        auto targetChannel = atoi(cmd.substr(5).c_str());
        joinChannel(targetChannel);
    }
#if ARDUINO_ARCH_ESP32 
    else if (cmd.rfind("noti ", 0) == 0)
    {
        Serial.println();
        if (_locked)        
        {
            Serial.println("Channel is locked");
            return true;
        }
        auto notificationNr = atoi(cmd.substr(5).c_str());
        playNotification(notificationNr);
    }
#endif
    else if (cmd.rfind("src ", 0) == 0)
    {
        Serial.println();
        if (_locked)        
        {
            Serial.println("Channel is locked");
            return true;
        }
        auto src = atoi(cmd.substr(4).c_str());
        auto& ko = KoSON_CHSourceNumber;
        ko.valueNoSend((uint8_t) src, DPT_Value_1_Ucount);
        processInputKo(ko);
    }
    else if (cmd.rfind("pl ", 0) == 0)
    {
        if (_locked)        
        {
            Serial.println("Channel is locked");
            return true;
        }
        auto playList = cmd.substr(2);
        _sonosSpeaker->playSonosPlaylist(playList.c_str());
    }
    else if (cmd == "test1")
    {
        if (_locked)        
        {
            Serial.println("Channel is locked");
            return true;
        }
        _sonosSpeaker->playInternetRadio("https://orf-live.ors-shoutcast.at/wie-q2a.m3u", "Radio Wien");
    }
    else if (cmd == "test2")
    {
        if (_locked)        
        {
            Serial.println("Channel is locked");
            return true;
        }
        _sonosSpeaker->playMusicLibraryDirectory("//192.168.0.1/Share/Storage/Musik/Violent Femmes/3");
    }
    else if (cmd == "test3")
    {
        if (_locked)        
        {
            Serial.println("Channel is locked");
            return true;
        }
        _sonosSpeaker->playMusicLibraryDirectory("//192.168.0.1/Share/Storage/Musik/Whippersnapper/Stories/");
    }
    else if (cmd == "test4")
    {
        if (_locked)        
        {
            Serial.println("Channel is locked");
            return true;
        }
        _sonosSpeaker->playMusicLibraryFile("//192.168.0.1/Share/Storage/Musik/Violent%20Femmes/3/01%20-%20Violent%20Femmes%20-%20Nightmares.mp3");
    }
    else if (cmd == "test5")
    {
        if (_locked)        
        {
            Serial.println("Channel is locked");
            return true;
        }
        _sonosSpeaker->stop();
    }
    else
        return false;
    return true;
}
#if ARDUINO_ARCH_ESP32 
void SonosChannel::playNotification(byte notificationNumber)
{
    if (_locked)
        return;

    auto _channelIndex = notificationNumber - 1;
    _sonosSpeaker->playNotification(ParamSONNOT_NotificationUrlStr.c_str(), ParamSONNOT_NotificationVolume);
}
#endif

void SonosChannel::joinNextPlayingGroup()
{
    if (_locked)
        return;
    delegateCoordination(true);

    auto groupCoordinator = _sonosSpeaker->findNextPlayingGroupCoordinator();
    if (groupCoordinator != nullptr)
    {
        logDebugP("Join with %s", groupCoordinator->getSpeakerIP().toString().c_str());
        _sonosSpeaker->joinToGroupCoordinator(groupCoordinator);
    }
    else
        logDebugP("No next playing group found");
}

bool SonosChannel::delegateCoordination(bool rejoinGroup)
{
    if (_locked)
        return false;
    auto currentGroupCoordinator = _sonosSpeaker->findGroupCoordinator(true);
    if (currentGroupCoordinator == _sonosSpeaker)
    {
        // delegate to other participant of the group
        auto firstParticpant = _sonosSpeaker->findFirstParticipant(true);
        if (firstParticpant != nullptr)
        {
            _sonosSpeaker->delegateGroupCoordinationTo(firstParticpant, rejoinGroup);
            return true;
        }
    }
    return false;
}


void SonosChannel::setVolumeRelative(int8_t relativeVolume)
{
    if (_locked)
        return;
    _sonosSpeaker->setVolumeRelative(relativeVolume);
}

void SonosChannel::setVolume(uint8_t volume)
{
    if (_locked)
        return;
    _sonosSpeaker->setVolume(volume);
}

void SonosChannel::setGroupVolumeRelative(int8_t relativeVolume)
{
    if (_locked)
        return;
    auto groupCoordinator = _sonosSpeaker->findGroupCoordinator();
    if (groupCoordinator != nullptr)
        groupCoordinator->setGroupVolumeRelative(relativeVolume);
}

void SonosChannel::setGroupVolume(uint8_t volume)
{
    if (_locked)
        return;
    auto groupCoordinator = _sonosSpeaker->findGroupCoordinator();
    if (groupCoordinator != nullptr)
        groupCoordinator->setGroupVolume(volume);
}

uint8_t SonosChannel::getGroupVolume()
{
    if (_locked)
        return 0;
    auto groupCoordinator = _sonosSpeaker->findGroupCoordinator();
    if (groupCoordinator != nullptr)
        return groupCoordinator->getGroupVolume();
    return 0;
}

void SonosChannel::togglePause()
{
    if (_locked)
        return;
    auto groupCoordinator = _sonosSpeaker->findGroupCoordinator();
    if (groupCoordinator != nullptr)
    {
        auto playState = groupCoordinator->getPlayState();
        if (playState == SonosApiPlayState::Playing)
            groupCoordinator->pause();
        else
            groupCoordinator->play();
    }
}

void SonosChannel::nextTrack()
{
    if (_locked)
        return;
    auto groupCoordinator = _sonosSpeaker->findGroupCoordinator();
    if (groupCoordinator != nullptr)
        groupCoordinator->next();
}

void SonosChannel::previousTrack()
{
    if (_locked)
        return;
    auto groupCoordinator = _sonosSpeaker->findGroupCoordinator();
    if (groupCoordinator != nullptr)
        groupCoordinator->previous();
}

SonosApiPlayState SonosChannel::getPlayState()
{
    if (_locked)
        return SonosApiPlayState::Stopped;
    auto groupCoordinator = _sonosSpeaker->findGroupCoordinator();
    if (groupCoordinator != nullptr)
        return groupCoordinator->getPlayState();
    return SonosApiPlayState::Stopped;
}

uint8_t SonosChannel::getChannelIndex()
{
   return _channelIndex;
}