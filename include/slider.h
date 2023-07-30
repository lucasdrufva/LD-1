#pragma once

// Slider control functions

void slider_init();

void slider_move(uint16_t);

void slider_register_callback(void (*cb)(uint16_t));

void slider_run();