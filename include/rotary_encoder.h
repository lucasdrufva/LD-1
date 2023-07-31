#pragma once 

#include <Arduino.h>

#define ROTARY_EVENT_RESET 0x01

struct RotaryEncoderConfig
{
    int start_index;
    int min_value;
    int max_value;
    bool circle;
};

void start_task_rotary_encoder();

extern int rotary_encoder_value;
extern struct RotaryEncoderConfig rotary_encoder_config;
extern TaskHandle_t rotary_encoder_task_handle;