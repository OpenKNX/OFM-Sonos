#include "LedFunctionSonosOnlineState.h"

#include "NetworkModule.h"
#include "SonosChannel.h"
#include "SonosModule.h"

void LedFunctionSonosOnlineState::loop()
{
    if (_ledFunctionGroup == nullptr)
    {
        _ledFunctionGroup = openknx.ledFunctions.get(OPENKNX_LEDFUNC_SONOS_ONLINE_STATE);
    }

    SonosOnlineState state = SonosOnlineState::AllOffline;
    if (openknxNetwork.established() && openknxSonosModule.isInitialized())
    {
        uint8_t usedChannels = 0;
        uint8_t onlineChannels = 0;
        for (uint8_t channelIndex = 0; channelIndex < openknxSonosModule.getNumberOfChannels(); channelIndex++)
        {
            auto channel = openknxSonosModule.getChannel(channelIndex);
            if (channel == nullptr)
                continue;

            usedChannels++;
            if (channel->isOnline())
                onlineChannels++;
        }

        if (usedChannels > 0 && onlineChannels == usedChannels)
            state = SonosOnlineState::AllOnline;
        else if (onlineChannels > 0)
            state = SonosOnlineState::PartiallyOffline;
    }

    if (state == _lastState)
        return;

    _lastState = state;
    switch (state)
    {
        case SonosOnlineState::AllOnline:
            _ledFunctionGroup->color(OpenKNX::Led::Color::Green);
            _ledFunctionGroup->on(OpenKNX::Led::Capability::COLOR);
            _ledFunctionGroup->on(OpenKNX::Led::Capability::MONOCHROME);
            break;
        case SonosOnlineState::PartiallyOffline:
            _ledFunctionGroup->color(OpenKNX::Led::Color::Yellow);
            _ledFunctionGroup->on(OpenKNX::Led::Capability::COLOR);
            _ledFunctionGroup->blinking(500, OpenKNX::Led::Capability::MONOCHROME);
            break;
        case SonosOnlineState::AllOffline:
            _ledFunctionGroup->color(OpenKNX::Led::Color::Red);
            _ledFunctionGroup->on(OpenKNX::Led::Capability::COLOR);
            _ledFunctionGroup->off(OpenKNX::Led::Capability::MONOCHROME);
            break;
    }
}
