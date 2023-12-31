#include "dmx.h"

#include <Arduino.h>
#include <esp_dmx.h>

#include "main.h"

int transmitPin = 17;
int receivePin = 16;
int enablePin = 19;

dmx_port_t dmxPort = 1;

byte dmx_data[DMX_PACKET_SIZE];

void dmx_init()
{
    dmx_set_pin(dmxPort, transmitPin, receivePin, enablePin);

    dmx_driver_install(dmxPort, DMX_DEFAULT_INTR_FLAGS);
}

void dmx_channels_to_dmx(struct ChannelInfo *channels)
{
    for (int i = 1; i < DMX_PACKET_SIZE; i++) {
      dmx_data[i] = map(channels[i-1].value, 0, CHANNEL_MAX_VALUE, 0, 255);
    }

    dmx_write(dmxPort, dmx_data, DMX_PACKET_SIZE);

    /* Now we can transmit the DMX packet! */
    dmx_send(dmxPort, DMX_PACKET_SIZE);
}

void dmx_run()
{
    dmx_send(dmxPort, DMX_PACKET_SIZE);
}