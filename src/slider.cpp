#include <Arduino.h>

#include "slider.h"

// Slider control functions

#define SLIDER_PIN_TOUCH T8
#define SLIDER_PIN_MOTOR_A 32
#define SLIDER_PIN_MOTOR_B 14
#define SLIDER_PIN_VALUE A0

#define SLIDER_MAX 4020
#define SLIDER_MIN 0
#define SLIDER_RANGE (SLIDER_MAX - SLIDER_MIN)

uint16_t target;
uint16_t current;
int output;

void (*slider_cb)(uint16_t new_value);

void slider_init()
{
    pinMode(SLIDER_PIN_MOTOR_A, OUTPUT);
    pinMode(SLIDER_PIN_MOTOR_B, OUTPUT);
    pinMode(SLIDER_PIN_VALUE, INPUT);
}

void slider_move(uint16_t new_target)
{
    target = new_target;
}

void slider_register_callback(void (*cb)(uint16_t))
{
    slider_cb = cb;
}

void slider_run()
{

    current = analogRead(SLIDER_PIN_VALUE);

    // Serial.print(target);
    // Serial.print(" : ");
    // Serial.println(current);

    if (touchRead(SLIDER_PIN_TOUCH) < 30)
    {
        analogWrite(SLIDER_PIN_MOTOR_A, 0);
        analogWrite(SLIDER_PIN_MOTOR_B, 0);
        if (current != target)
        {
            if (slider_cb != nullptr)
            {
                slider_cb(current);
            }
        }
        return;
    }

    output = min(abs(current - target) * 0.11, 150.0) + 10;

    if (current - target > SLIDER_RANGE * 0.02)
    {
        analogWrite(SLIDER_PIN_MOTOR_A, output);
        analogWrite(SLIDER_PIN_MOTOR_B, 0);
    }
    else if (target - current > SLIDER_RANGE * 0.02)
    {
        analogWrite(SLIDER_PIN_MOTOR_A, 0);
        analogWrite(SLIDER_PIN_MOTOR_B, output);
    }
    else
    {
        analogWrite(SLIDER_PIN_MOTOR_A, 0);
        analogWrite(SLIDER_PIN_MOTOR_B, 0);
    }
}