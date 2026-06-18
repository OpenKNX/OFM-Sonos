#pragma once
#include "OpenKNX.h"
#include "ChannelOwnerModule.h"
#include "LedFunctionSonosOnlineState.h"
#include "SonosApi.h"
#if USE_ESP_ASNC_WEB_SERVER 
#include <ESPAsyncWebServer.h>
#endif
#ifndef OPENKNX_DUALCORE
#error Sonos Module requires OPENKNX_DUALCORE
#endif
#ifndef OPENKNX_PING
#error "Sonos Module requires OPENKNX_PING"
#endif
class SonosChannel;
class ConsoleWriterStream;

class SonosModule : public SonosChannelOwnerModule
{
    ConsoleWriterStream* _debugWriter = nullptr;
    SonosApi _sonosApi;
    LedFunctionSonosOnlineState _statusLedSonosOnlineState;
    bool _channelSetupCalled = false;
    volatile bool _channelSetup1Called = false;
    bool _networkConnected = false;
  public:

    void setup() override;
    void setup1() override;

    const std::string name() override;
    const std::string version() override;
    void processInputKo(GroupObject &ko) override;
    void processBeforeRestart() override;
    void processBeforeTablesUnload() override;
    void showInformations() override;
    void savePower() override;
    bool restorePower() override;
    void loop() override;
    void loop1() override;
    void showHelp() override;
    bool processCommand(const std::string cmd, bool diagnoseKo) override;
    SonosChannel* getChannel(uint8_t channelIndex);
    bool isInitialized();

  protected:
    OpenKNX::Channel* createChannel(uint8_t _channelIndex /* this parameter is used in macros, do not rename */) override; 
};

extern SonosModule openknxSonosModule;