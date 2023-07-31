#pragma once

#include <Arduino.h>

#include "main.h"

#define DISPLAY_EVENT_UPDATE 0x01
#define DISPLAY_MENU_MAX_DRAWN 5
#define DISPLAY_BACKGROUND_COLOR 0x0A2F

enum DisplayContentType { channel, menu };

struct DisplayMenuOption
{
    char title[16];
};

struct DisplayMenu
{
    uint8_t selected;
    uint8_t length;
    struct DisplayMenuOption options[16];
};

struct DisplayContent
{
    DisplayContentType type;
    union
    {
        struct ChannelInfo channel_info;
        struct DisplayMenu menu;
    };
};

void start_task_display();

extern DisplayContent display_content;
extern SemaphoreHandle_t display_content_mutex;
extern TaskHandle_t display_task_handle;