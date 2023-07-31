#include "dmx.h"

#include <Arduino.h>
#include <esp_dmx.h>

#include "main.h"

/* First, lets define the hardware pins that we are using with our ESP32. We
  need to define which pin is transmitting data and which pin is receiving data.
  DMX circuits also often need to be told when we are transmitting and when we
  are receiving data. We can do this by defining an enable pin. */
int transmitPin = 17;
int receivePin = 16;
int enablePin = 19;
/* Make sure to double-check that these pins are compatible with your ESP32!
  Some ESP32s, such as the ESP32-WROVER series, do not allow you to read or
  write data on pins 16 or 17, so it's always good to read the manuals. */

/* Next, lets decide which DMX port to use. The ESP32 has either 2 or 3 ports.
  Port 0 is typically used to transmit serial data back to your Serial Monitor,
  so we shouldn't use that port. Lets use port 1! */
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