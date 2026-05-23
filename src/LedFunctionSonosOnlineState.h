#pragma once
#include "OpenKNX.h"

#define OPENKNX_LEDFUNC_SONOS_ONLINE_STATE 405

enum class SonosOnlineState : uint8_t
{
    AllOffline = 0,
    PartiallyOffline = 1,
    AllOnline = 2,
};

class LedFunctionSonosOnlineState
{
#ifdef OPENKNX_LEDFUNC_BASE_TIME
    OpenKNX::Led::FunctionGroup* _ledFunctionGroup = nullptr;
    SonosOnlineState _lastState = (SonosOnlineState)255;
#endif
public:
    void loop();
};
