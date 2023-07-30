#include "button.h"

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define BUTTON_PIN_KEY 25 //A1
#define BUTTON_PIN_NEOPIXEL 13

void (*button_cb)(bool new_state);

bool state = false;

Adafruit_NeoPixel pixel(1, BUTTON_PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

void button_init()
{
    pinMode(BUTTON_PIN_KEY, INPUT);
    pixel.begin();

}

void button_register_callback(void (*cb)(bool))
{
    button_cb = cb;
}

void button_run()
{
    bool new_state = digitalRead(BUTTON_PIN_KEY);
    if(new_state != state)
    {
        state = new_state;
        if(button_cb != nullptr)
        {
            button_cb(new_state);
        }
    }

    if(state)
    {
        pixel.setPixelColor(0, pixel.Color(50,50,200)); 
    }
    else
    {
        pixel.setPixelColor(0, pixel.Color(0,0,10)); 
    }
    pixel.show();
}