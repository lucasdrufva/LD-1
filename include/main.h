#pragma once

#include <Arduino.h>

#define MAIN_EVENT_ROTARY_CHANGED 0x01
#define MAIN_EVENT_ROTARY_BUTTON_CLICK 0x02

struct ChannelInfo
{
    int index;
    uint16_t value;
    char name[16];
    uint16_t color;
    bool is_virtual;
};

extern TaskHandle_t main_task_handle;