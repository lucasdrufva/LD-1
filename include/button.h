#pragma once

void button_init();

void button_register_callback(void (*cb)(bool));

void button_run();