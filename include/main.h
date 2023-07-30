#pragma once

#include <Arduino.h>

#define MAIN_EVENT_CHANGE_CHANNEL 0x01

extern int current_channel;
extern TaskHandle_t main_task_handle;