#pragma once

#include <Arduino.h>

#define DISPLAY_EVENT_UPDATE 0x01

enum DisplayContentType { channel, menu };

struct DisplayChannelInfo
{
    int number;
    char name[16];
    uint16_t color;
};

struct DisplayMenuOption
{
    char name[16];
};

struct DisplayMenu
{
    struct DisplayMenuOption options[5];
};

struct DisplayContent
{
    DisplayContentType type;
    union
    {
        struct DisplayChannelInfo channel_info;
        struct DisplayMenu menu;
    };
};

void start_task_display();

extern DisplayContent display_content;
extern SemaphoreHandle_t display_content_mutex;
extern TaskHandle_t display_task_handle;