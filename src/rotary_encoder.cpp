#include "rotary_encoder.h"

#include <Arduino.h>
#include "AiEsp32RotaryEncoder.h"

#include "main.h"

#define ROTARY_ENCODER_A_PIN 15
#define ROTARY_ENCODER_B_PIN 27
#define ROTARY_ENCODER_BUTTON_PIN A2
#define ROTARY_ENCODER_VCC_PIN -1
#define ROTARY_ENCODER_STEPS 4

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

TaskHandle_t rotary_encoder_task_handle;

int rotary_encoder_value;
struct RotaryEncoderConfig rotary_encoder_config = {1, 1, 512, false};

void (*rotary_cb)(uint32_t new_value);

void rotary_encoder_register_callback(void (*cb)(uint32_t))
{
  rotary_cb = cb;
}

void IRAM_ATTR readEncoderISR()
{
  rotaryEncoder.readEncoder_ISR();
}

void rotary_encoder_init()
{
  // we must initialize rotary encoder
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);

  bool circleValues = false;
  rotaryEncoder.setBoundaries(0, 512, circleValues);
  rotaryEncoder.setEncoderValue(1);

  rotaryEncoder.setAcceleration(50);
}

void rotary_encoder_task(void *parameter)
{
  rotary_encoder_init();

  uint32_t ulNotifiedValue;

  for (;;)
  {
    xTaskNotifyWait(
        0x00,
        ULONG_MAX,
        &ulNotifiedValue,
        0);

    if ((ulNotifiedValue & ROTARY_EVENT_RESET) != 0)
    {
      rotaryEncoder.setEncoderValue(rotary_encoder_config.start_index);
      rotaryEncoder.setBoundaries(rotary_encoder_config.min_value, rotary_encoder_config.max_value, rotary_encoder_config.circle);
    }

    if (rotaryEncoder.encoderChanged())
    {
      rotary_encoder_value = rotaryEncoder.readEncoder();
      xTaskNotify(main_task_handle, MAIN_EVENT_ROTARY_CHANGED, eSetValueWithOverwrite);
    }

    if (rotaryEncoder.isEncoderButtonClicked())
    {
      xTaskNotify(main_task_handle, MAIN_EVENT_ROTARY_BUTTON_CLICK, eSetValueWithOverwrite);
    }
  }
}

void start_task_rotary_encoder()
{
  xTaskCreate(rotary_encoder_task, "rotary_encoder_task", 1024, NULL, 1, &rotary_encoder_task_handle);
}

extern uint16_t num_virtual_channels;

void rotary_encoder_set_mode_channels(int current)
{
  rotary_encoder_config.min_value = 0-num_virtual_channels;
  rotary_encoder_config.max_value = 512;
  rotary_encoder_config.circle = false;
  rotary_encoder_config.start_index = current;

  xTaskNotify(rotary_encoder_task_handle, ROTARY_EVENT_RESET, eSetValueWithOverwrite);
}