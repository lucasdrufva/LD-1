#pragma once
#include <Arduino.h>

void dmx_init();

void dmx_channels_to_dmx(struct ChannelInfo *channels);

void dmx_run();