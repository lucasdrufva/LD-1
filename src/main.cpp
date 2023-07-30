#include <Arduino.h>

#include "slider.h"
#include "rotary_encoder.h"
#include "display.h"
#include "dmx.h"
#include "button.h"

uint16_t slider_value = 0;
uint16_t before_flash_slider_value = 0;

int current_channel = 1;
uint16_t channel_values[512];

void slider_updated(uint16_t new_value)
{
    channel_values[current_channel-1] = new_value;
    slider_move(new_value);
    dmx_channel_values_to_dmx(channel_values);
}

void rotary_updated(uint32_t new_value)
{
    current_channel = new_value;
    slider_move(channel_values[current_channel-1]);

    xSemaphoreTake(display_content_mutex, portMAX_DELAY);
    display_content.type = DisplayContentType::channel;
    display_content.channel_info.number = current_channel;
    xSemaphoreGive(display_content_mutex);

    xTaskNotify(display_task_handle, DISPLAY_EVENT_UPDATE, eSetValueWithOverwrite);
}

void button_updated(bool new_state)
{
    if(new_state == HIGH)
    {
        before_flash_slider_value = channel_values[current_channel-1];
        channel_values[current_channel-1] = 65535;
    }
    else
    {
        channel_values[current_channel-1] = before_flash_slider_value;
    }
    slider_move(channel_values[current_channel-1]);
    dmx_channel_values_to_dmx(channel_values);
}

void setup()
{
    Serial.begin(9600);

    display_content_mutex = xSemaphoreCreateMutex();

    slider_init();
    slider_register_callback(&slider_updated);
    slider_move(slider_value);

    rotary_encoder_init();
    rotary_encoder_register_callback(&rotary_updated);

    start_task_display();

    dmx_init();

    button_init();
    button_register_callback(&button_updated);
}

void loop()
{
    Serial.print(current_channel);
    Serial.print(" : ");
    Serial.println(channel_values[current_channel-1]);

    rotary_encoder_run();
    slider_run();
    dmx_run();
    button_run();
}