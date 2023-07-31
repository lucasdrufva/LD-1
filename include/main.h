#pragma once

#include <Arduino.h>

#define MAIN_EVENT_ROTARY_CHANGED 0x01
#define MAIN_EVENT_ROTARY_BUTTON_CLICK 0x02

#define CHANNEL_MAX_VALUE 4095

struct ChannelProgramming
{
    int index;
    uint16_t value;
};

struct ChannelInfo
{
    int index;
    uint16_t value;
    char name[16];
    uint16_t color;
    bool is_virtual;
    struct ChannelProgramming programmings[4]; //TODO: store these in a more effective way
};

enum MenuState
{
    closed,
    channel_main,
    channel_color,
    create_vChannel,
    programming
};

extern TaskHandle_t main_task_handle;
extern MenuState menu_state;