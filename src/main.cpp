#include "main.h"

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

TaskHandle_t main_task_handle;

void slider_updated(uint16_t new_value)
{
    channel_values[current_channel-1] = new_value;
    slider_move(new_value);
    dmx_channel_values_to_dmx(channel_values);
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

void main_handle_change_channel()
{
    //Update slider
    slider_move(channel_values[current_channel-1]);

    //Update display
    xSemaphoreTake(display_content_mutex, portMAX_DELAY);
    display_content.type = DisplayContentType::channel;
    display_content.channel_info.number = current_channel;
    xSemaphoreGive(display_content_mutex);

    xTaskNotify(display_task_handle, DISPLAY_EVENT_UPDATE, eSetValueWithOverwrite);
}

void main_task(void *parameter)
{
    uint32_t ulNotifiedValue;

    for(;;)
    {
        xTaskNotifyWait(
            0x00,
            ULONG_MAX,
            &ulNotifiedValue,
            10 / portTICK_PERIOD_MS);

        if ((ulNotifiedValue & MAIN_EVENT_CHANGE_CHANNEL) != 0)
        {
            main_handle_change_channel();
        }
    }
}

void start_task_main()
{
    xTaskCreate(main_task, "main_task", 1024, NULL, 1, &main_task_handle);
}

void setup()
{
    Serial.begin(9600);

    display_content_mutex = xSemaphoreCreateMutex();

    slider_init();
    slider_register_callback(&slider_updated);
    slider_move(slider_value);

    start_task_display();
    start_task_rotary_encoder();

    dmx_init();

    button_init();
    button_register_callback(&button_updated);
}

void loop()
{
    Serial.print(current_channel);
    Serial.print(" : ");
    Serial.println(channel_values[current_channel-1]);

    slider_run();
    dmx_run();
    button_run();
}