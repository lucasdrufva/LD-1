#pragma once
#include <Arduino.h>

void dmx_init();

void dmx_channel_values_to_dmx(uint16_t *channel_values);

void dmx_run();