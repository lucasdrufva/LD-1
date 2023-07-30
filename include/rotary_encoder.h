#pragma once 

void rotary_encoder_init();

void rotary_encoder_register_callback(void (*cb)(uint32_t));

void rotary_encoder_run();