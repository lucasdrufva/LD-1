#include "display.h"

#include <Arduino.h>
#include "Adafruit_ST7789.h"

#define DISPLAY_PIN_CS 21
#define DISPLAY_PIN_RST 4
#define DISPLAY_PIN_DC 12

Adafruit_ST7789 tft = Adafruit_ST7789(DISPLAY_PIN_CS, DISPLAY_PIN_DC, 18, 5, DISPLAY_PIN_RST);

TaskHandle_t display_task_handle;
SemaphoreHandle_t display_content_mutex;
struct DisplayContent display_content;

DisplayContentType previous_type;

void display_init()
{
    pinMode(DISPLAY_PIN_CS, OUTPUT);
    pinMode(DISPLAY_PIN_RST, OUTPUT);
    pinMode(DISPLAY_PIN_DC, OUTPUT);

    tft.init(240, 240);
    tft.setRotation(2);
    tft.fillScreen(DISPLAY_BACKGROUND_COLOR);
}

void display_channel(struct ChannelInfo channel)
{
    char cstr[16];
    itoa(channel.index, cstr, 10);

    // TODO handle numbers more than 1 character long better
    tft.setCursor(110, 110);
    tft.setTextSize(5);
    tft.setTextColor(tft.color565(255, 255, 255));
    tft.fillRect(110,110, 100,40, DISPLAY_BACKGROUND_COLOR);
    if(channel.is_virtual)
    {
        tft.print("v");
    }
    
    tft.println(cstr);
    if(channel.color)
    {
        Serial.println(channel.color);
        tft.fillRoundRect(180,180, 30,30, 5, channel.color);
    }
    else
    {
        tft.fillRoundRect(180,180, 30,30, 5, DISPLAY_BACKGROUND_COLOR);
    }
}

void display_menu(struct DisplayMenu menu)
{
    int start = menu.selected - 1;
    if (start < 0)
        start = 0;

    int end = start + DISPLAY_MENU_MAX_DRAWN;
    if (end > menu.length)
    {
        end = menu.length;
    }

    tft.fillScreen(DISPLAY_BACKGROUND_COLOR);
    tft.setCursor(0, 0);
    tft.setTextSize(5);

    for (int i = start; i < end; i++)
    {
        if (i == menu.selected)
        {
            tft.print(">");
        }
        tft.print(" ");
        tft.println(menu.options[i].title);
    }
}

void display_prompt(struct DisplayPrompt prompt)
{
    tft.fillScreen(DISPLAY_BACKGROUND_COLOR);
    tft.setCursor(0, 0);
    tft.setTextSize(4);
    tft.setTextWrap(true);
    tft.println(prompt.prompt);
    tft.setTextWrap(false);

    int start = prompt.selected - 1;
    if (start < 0)
        start = 0;

    int end = start + DISPLAY_MENU_MAX_DRAWN-1;
    if (end > prompt.length)
    {
        end = prompt.length;
    }

    tft.setCursor(0, 120);
    tft.setTextSize(3);

    for (int i = start; i < end; i++)
    {
        if (i == prompt.selected)
        {
            tft.print(">");
        }
        tft.print(" ");
        tft.println(prompt.options[i].title);
    }
}

void display_handle_update()
{
    struct DisplayContent content;

    xSemaphoreTake(display_content_mutex, portMAX_DELAY);
    content = display_content;
    xSemaphoreGive(display_content_mutex);

    if(content.type != previous_type)
    {
        tft.fillScreen(DISPLAY_BACKGROUND_COLOR);
    }

    previous_type = content.type;

    if (content.type == DisplayContentType::channel)
    {
        display_channel(content.channel_info);
    }
    else if (content.type == DisplayContentType::menu)
    {
        display_menu(content.menu);
    }
    else if (content.type == DisplayContentType::prompt)
    {
        display_prompt(content.prompt);
    }
}

int animation_state;
unsigned long animation_timer;

void display_task(void *parameter)
{
    display_init();

    vTaskDelay(100 / portTICK_PERIOD_MS);

    uint32_t ulNotifiedValue;

    for (;;)
    {
        xTaskNotifyWait(
            0x00,
            ULONG_MAX,
            &ulNotifiedValue,
            10 / portTICK_PERIOD_MS);

        if ((ulNotifiedValue & DISPLAY_EVENT_UPDATE) != 0)
        {
            display_handle_update();
        }

        if(menu_state == MenuState::programming)
        {
            if(millis() - animation_timer > 1000)
            {
                if(animation_state)
                {
                    tft.fillCircle(40, 200, 10, tft.color565(255, 0,0));
                }
                else
                {
                    tft.fillCircle(40, 200, 10, DISPLAY_BACKGROUND_COLOR);
                }
                animation_state = !animation_state;
                animation_timer = millis();
            }
        }
    }
}

void start_task_display()
{
    xTaskCreate(display_task, "display_task", 21000, NULL, 1, &display_task_handle);
}