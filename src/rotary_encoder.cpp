#include "rotary_encoder.h"

#include <Arduino.h>
#include "AiEsp32RotaryEncoder.h"

#include "main.h"

#define ROTARY_ENCODER_A_PIN 15
#define ROTARY_ENCODER_B_PIN 27
#define ROTARY_ENCODER_BUTTON_PIN 13
#define ROTARY_ENCODER_VCC_PIN -1
#define ROTARY_ENCODER_STEPS 4

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

TaskHandle_t rotary_encoder_task_handle;

void (*rotary_cb)(uint32_t new_value);

void rotary_encoder_register_callback(void (*cb)(uint32_t))
{
    rotary_cb = cb;
}

void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR();
}

void rotary_encoder_init()
{
    //we must initialize rotary encoder
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);

  bool circleValues = false;
  rotaryEncoder.setBoundaries(1, 512, circleValues);

  rotaryEncoder.setAcceleration(50);

}

void rotary_encoder_task(void *parameter)
{
  rotary_encoder_init();

  for(;;)
  {
    if(rotaryEncoder.encoderChanged()){
        current_channel = rotaryEncoder.readEncoder();
        xTaskNotify(main_task_handle, MAIN_EVENT_CHANGE_CHANNEL, eSetValueWithOverwrite);
    }
  }
}

void start_task_rotary_encoder()
{
    xTaskCreate(rotary_encoder_task, "rotary_encoder_task", 1024, NULL, 1, &rotary_encoder_task_handle);
}