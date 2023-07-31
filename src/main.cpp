#include "main.h"

#include <Arduino.h>

#include "slider.h"
#include "rotary_encoder.h"
#include "display.h"
#include "dmx.h"
#include "button.h"

const uint16_t COLORS[] = {
    0xE8A2, // RED
    0xFD20, // ORANGE
    0xFF46, // YELLOW
    0x7E02, // GREEN
    0x4BFC, // BLUE
    0x49B3, // INDIGO
    0x71B3, // VIOLET
    0xFFFF, // WHITE
};

uint16_t slider_value = 0;
uint16_t before_flash_slider_value = 0;

int current_channel = 1;
int programming_channel;
// uint16_t channel_values[512];

ChannelInfo channels_data[1024];
ChannelInfo *channels = &channels_data[512];

uint16_t num_virtual_channels = 0;

TaskHandle_t main_task_handle;

MenuState menu_state = closed;
int menu_index;

void slider_updated(uint16_t new_value)
{
    if (menu_state == MenuState::programming)
    {
        int channel_to_program = 0;
        for (int i = 0; i < 4; i++)
        {
            if (channels[programming_channel].programmings[i].index == 0 || channels[programming_channel].programmings[i].index == current_channel)
            {
                channel_to_program = i;
                break;
            }
        }
        channels[programming_channel].programmings[channel_to_program].index = current_channel;
        channels[programming_channel].programmings[channel_to_program].value = new_value;

        Serial.print("Current channel: ");
        Serial.print(current_channel);
        Serial.print(" - Programmed channel ");
        Serial.print(channels[programming_channel].programmings[channel_to_program].index);
        Serial.print(" to value ");
        Serial.print(channels[programming_channel].programmings[channel_to_program].value);
        Serial.print(" on ");
        Serial.println(channel_to_program);
    }
    else
    {
        channels[current_channel - 1].value = new_value;
    }

    if (channels[current_channel - 1].is_virtual)
    {
        for (int i = 0; i < 4; i++)
        {
            if (channels[programming_channel].programmings[i].index != 0)
            {
                uint16_t calculated_value = ((double)((double)new_value) / (double)CHANNEL_MAX_VALUE) * channels[programming_channel].programmings[i].value;
                channels[channels[programming_channel].programmings[i].index - 1].value = calculated_value;
                Serial.print("Index: ");
                Serial.print(channels[programming_channel].programmings[i].index);
                Serial.print(" Value: ");
                Serial.println(calculated_value);
            }
        }
    }

    slider_move(new_value);
    dmx_channels_to_dmx(channels);
}

void button_updated(bool new_state)
{
    if (new_state == HIGH)
    {
        before_flash_slider_value = channels[current_channel - 1].value; // channel_values[current_channel - 1];
        // channel_values[current_channel - 1] = 65535;
    }
    else
    {
        channels[current_channel - 1].value = before_flash_slider_value;
        // channel_values[current_channel - 1] = before_flash_slider_value;
    }
    slider_move(channels[current_channel - 1].value);
    dmx_channels_to_dmx(channels);
}

void main_prompt_create_vChannel()
{
    menu_state = MenuState::create_vChannel;

    // Ask if user wants to create new virtual channel
    // Update display
    xSemaphoreTake(display_content_mutex, portMAX_DELAY);
    display_content.type = DisplayContentType::prompt;
    display_content.prompt.selected = 0;
    menu_index = 0;
    display_content.prompt.length = 2;
    strcpy(display_content.prompt.prompt, "Create new vChannel?");
    strcpy(display_content.prompt.options[0].title, "Yes");
    strcpy(display_content.prompt.options[1].title, "No");
    xSemaphoreGive(display_content_mutex);

    xTaskNotify(display_task_handle, DISPLAY_EVENT_UPDATE, eSetValueWithOverwrite);

    // Update rotary encoder
    rotary_encoder_config.min_value = 0;
    rotary_encoder_config.max_value = 1;
    rotary_encoder_config.circle = true;
    rotary_encoder_config.start_index = 0;

    xTaskNotify(rotary_encoder_task_handle, ROTARY_EVENT_RESET, eSetValueWithOverwrite);
}

void main_handle_change_channel(int new_channel)
{
    if (new_channel < 1 - num_virtual_channels)
    {
        main_prompt_create_vChannel();

        return;
    }

    current_channel = new_channel;

    // Update slider
    if (menu_state == MenuState::programming)
    {
        slider_move(0);
    }
    else
    {
        slider_move(channels[current_channel - 1].value);
    }

    // Update display
    xSemaphoreTake(display_content_mutex, portMAX_DELAY);
    display_content.type = DisplayContentType::channel;
    display_content.channel_info = channels[current_channel - 1];
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
    strcpy(display_content.menu.options[0].title, "Color");
    if (channels[current_channel - 1].is_virtual)
    {
        display_content.menu.length = 3;
        strcpy(display_content.menu.options[1].title, "Program");
        strcpy(display_content.menu.options[2].title, "Close");
    }
    else
    {
        display_content.menu.length = 2;
        strcpy(display_content.menu.options[1].title, "Close");
    }

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

void main_start_programming()
{
    menu_state = MenuState::programming;

    programming_channel = current_channel;

    // Inform user that programming has started
    xSemaphoreTake(display_content_mutex, portMAX_DELAY);
    display_content.type = DisplayContentType::prompt;
    display_content.prompt.selected = 0;
    menu_index = 0;
    display_content.prompt.length = 0;
    strcpy(display_content.prompt.prompt, "Start programming");
    xSemaphoreGive(display_content_mutex);

    xTaskNotify(display_task_handle, DISPLAY_EVENT_UPDATE, eSetValueWithOverwrite);

    vTaskDelay(5000 / portTICK_PERIOD_MS);

    main_handle_change_channel(1);
    rotary_encoder_set_mode_channels(current_channel);
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
        else if (menu_index == 1 && channels[current_channel - 1].is_virtual)
        {
            main_start_programming();
        }
        else
        {
            main_handle_change_channel(current_channel);
            menu_state = MenuState::closed;

            rotary_encoder_set_mode_channels(current_channel);
        }
    }

    else if (menu_state == MenuState::channel_color)
    {
        if (menu_index == 8) // Close
        {
            // Hmmm
        }
        else
        {
            channels[current_channel - 1].color = COLORS[menu_index];
        }

        main_handle_change_channel(current_channel);
        menu_state = MenuState::closed;

        rotary_encoder_set_mode_channels(current_channel);
    }
    else if (menu_state == MenuState::create_vChannel)
    {
        if (menu_index == 0) // Yes create new vChannel
        {
            num_virtual_channels++;
            channels[-num_virtual_channels].index = num_virtual_channels;
            channels[-num_virtual_channels].is_virtual = true;

            current_channel = 1 - num_virtual_channels;

            main_start_programming();
        }
        else // Close
        {
            main_handle_change_channel(current_channel);
            menu_state = MenuState::closed;

            rotary_encoder_set_mode_channels(current_channel);
        }
    }
    else if (menu_state == MenuState::programming)
    {
        main_handle_change_channel(1);
        menu_state = MenuState::closed;

        rotary_encoder_set_mode_channels(1);

        for (int i = 0; i < 4; i++)
        {
            Serial.print("Finished programmed channel ");
            Serial.print(channels[programming_channel].programmings[i].index);
            Serial.print(" to value ");
            Serial.println(channels[programming_channel].programmings[i].value);
        }
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
            if (menu_state == MenuState::closed || menu_state == MenuState::programming)
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

    for (int i = 0; i < 512; i++)
    {
        channels[i].index = i + 1;
    }
}

void loop()
{
    // Serial.print(current_channel);
    // Serial.print(" : ");
    // Serial.println(channels[current_channel-1].value);

    slider_run();
    dmx_run();
    button_run();
}