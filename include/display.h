#pragma once

#include <Arduino.h>

#include "main.h"

#define DISPLAY_EVENT_UPDATE 0x01
#define DISPLAY_MENU_MAX_DRAWN 5
#define DISPLAY_BACKGROUND_COLOR 0x0A2F

enum DisplayContentType { channel, menu, prompt };

struct DisplayMenuOption
{
    char title[16];
};

struct DisplayPrompt
{
    uint8_t selected; //Selected needs to be first to match menu struct
    char prompt[32]; 
    uint8_t length;
    struct DisplayMenuOption options[4];
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
        struct DisplayPrompt prompt;
    };
};

void start_task_display();

extern DisplayContent display_content;
extern SemaphoreHandle_t display_content_mutex;
extern TaskHandle_t display_task_handle;