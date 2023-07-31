#include "main.h"

#include <Arduino.h>

#include "slider.h"
#include "rotary_encoder.h"
#include "display.h"
#include "dmx.h"
#include "button.h"

const uint16_t COLORS[] = {
    0xE8A2, //RED
    0xFD20, //ORANGE
    0xFF46, //YELLOW
    0x7E02, //GREEN
    0x4BFC, //BLUE
    0x49B3, //INDIGO
    0x71B3, //VIOLET
    0xFFFF, //WHITE
};

uint16_t slider_value = 0;
uint16_t before_flash_slider_value = 0;

int current_channel = 1;
//uint16_t channel_values[512];

ChannelInfo channels[1024];

TaskHandle_t main_task_handle;

enum MenuState
{
    closed,
    channel_main,
    channel_color
};
MenuState menu_state = closed;
int menu_index;

void slider_updated(uint16_t new_value)
{
    channels[current_channel - 1].value = new_value;
    slider_move(new_value);
    dmx_channels_to_dmx(channels);
}

void button_updated(bool new_state)
{
    if (new_state == HIGH)
    {
        before_flash_slider_value = channels[current_channel - 1].value; //channel_values[current_channel - 1];
        //channel_values[current_channel - 1] = 65535;
    }
    else
    {
        channels[current_channel - 1].value = before_flash_slider_value;
        //channel_values[current_channel - 1] = before_flash_slider_value;
    }
    slider_move(channels[current_channel - 1].value);
    dmx_channels_to_dmx(channels);
}

void main_handle_change_channel(int new_channel)
{
    current_channel = new_channel;

    // Update slider
    slider_move(channels[current_channel - 1].value);

    // Update display
    xSemaphoreTake(display_content_mutex, portMAX_DELAY);
    display_content.type = DisplayContentType::channel;
    //TODO whole channel should be sent instead
    display_content.channel_info.index = current_channel;
    display_content.channel_info.color = channels[current_channel-1].color;
    xSemaphoreGive(display_content_mutex);

    xTaskNotify(display_task_handle, DISPLAY_EVENT_UPDATE, eSetValueWithOverwrite);
}

void main_start_channel_menu()
{
    // Update display
    xSemaphoreTake(display_content_mutex, portMAX_DELAY);
    display_content.type = DisplayContentType::menu;
    display_content.menu.selected = 0;
    menu_index = 0;
    display_content.menu.length = 2;
    strcpy(display_content.menu.options[0].title, "Color");
    strcpy(display_content.menu.options[1].title, "Close");
    xSemaphoreGive(display_content_mutex);

    xTaskNotify(display_task_handle, DISPLAY_EVENT_UPDATE, eSetValueWithOverwrite);

    // Update rotary encoder
    rotary_encoder_config.min_value = 0;
    rotary_encoder_config.max_value = 1;
    rotary_encoder_config.circle = true;
    rotary_encoder_config.start_index = 0;

    xTaskNotify(rotary_encoder_task_handle, ROTARY_EVENT_RESET, eSetValueWithOverwrite);
}

void main_start_channel_color_menu()
{
    // Update display
    xSemaphoreTake(display_content_mutex, portMAX_DELAY);
    display_content.type = DisplayContentType::menu;
    display_content.menu.selected = 0;
    menu_index = 0;
    display_content.menu.length = 9;
    strcpy(display_content.menu.options[0].title, "Red");
    strcpy(display_content.menu.options[1].title, "Orange");
    strcpy(display_content.menu.options[2].title, "Yellow");
    strcpy(display_content.menu.options[3].title, "Green");
    strcpy(display_content.menu.options[4].title, "Blue");
    strcpy(display_content.menu.options[5].title, "Indigo");
    strcpy(display_content.menu.options[6].title, "Violet");
    strcpy(display_content.menu.options[7].title, "White");
    strcpy(display_content.menu.options[8].title, "Close");
    xSemaphoreGive(display_content_mutex);

    xTaskNotify(display_task_handle, DISPLAY_EVENT_UPDATE, eSetValueWithOverwrite);

    // Update rotary encoder
    rotary_encoder_config.min_value = 0;
    rotary_encoder_config.max_value = 8;
    rotary_encoder_config.circle = true;
    rotary_encoder_config.start_index = 0;

    xTaskNotify(rotary_encoder_task_handle, ROTARY_EVENT_RESET, eSetValueWithOverwrite);
}

void main_menu_set_index(uint8_t new_index)
{
    menu_index = new_index;

    xSemaphoreTake(display_content_mutex, portMAX_DELAY);
    display_content.menu.selected = new_index;
    xSemaphoreGive(display_content_mutex);

    xTaskNotify(display_task_handle, DISPLAY_EVENT_UPDATE, eSetValueWithOverwrite);
}

void main_handle_rotary_button_click()
{
    if (menu_state == MenuState::closed)
    {
        main_start_channel_menu();
        menu_state = MenuState::channel_main;
    }
    else if (menu_state == MenuState::channel_main)
    {
        if (menu_index == 0)
        {
            main_start_channel_color_menu();
            menu_state = MenuState::channel_color;
        }
        else if (menu_index == 1)
        {
            main_handle_change_channel(current_channel);
            menu_state = MenuState::closed;

            // Update rotary encoder
            rotary_encoder_config.min_value = 1;
            rotary_encoder_config.max_value = 512;
            rotary_encoder_config.circle = false;
            rotary_encoder_config.start_index = current_channel;

            xTaskNotify(rotary_encoder_task_handle, ROTARY_EVENT_RESET, eSetValueWithOverwrite);
        }
    }
    else if(menu_state == MenuState::channel_color)
    {
        if(menu_index == 8) // Close
        {
            //Hmmm
        }
        else
        {
            channels[current_channel-1].color = COLORS[menu_index];
        }

        main_handle_change_channel(current_channel);
        menu_state = MenuState::closed;
    }
}

void main_task(void *parameter)
{
    uint32_t ulNotifiedValue;

    for (;;)
    {
        xTaskNotifyWait(
            0x00,
            ULONG_MAX,
            &ulNotifiedValue,
            10 / portTICK_PERIOD_MS);

        if ((ulNotifiedValue & MAIN_EVENT_ROTARY_CHANGED) != 0)
        {
            if (menu_state == MenuState::closed)
            {
                main_handle_change_channel(rotary_encoder_value);
            }
            else
            {
                main_menu_set_index(rotary_encoder_value);
            }
        }

        if ((ulNotifiedValue & MAIN_EVENT_ROTARY_BUTTON_CLICK) != 0)
        {
            main_handle_rotary_button_click();
        }
    }
}

void start_task_main()
{
    xTaskCreate(main_task, "main_task", 1024, NULL, 1, &main_task_handle);
}

void setup()
{
    channels[0].color = 0;
    channels[1].color = 0;
    channels[2].color = 0;

    Serial.begin(9600);

    display_content_mutex = xSemaphoreCreateMutex();

    slider_init();
    slider_register_callback(&slider_updated);
    slider_move(slider_value);

    start_task_main();
    start_task_display();
    start_task_rotary_encoder();

    dmx_init();

    button_init();
    button_register_callback(&button_updated);
}

void loop()
{
    // Serial.print(current_channel);
    // Serial.print(" : ");
    // Serial.println(channel_values[current_channel-1]);

    slider_run();
    dmx_run();
    button_run();
}