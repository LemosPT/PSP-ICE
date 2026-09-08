#ifndef PSP_ICE_INPUT_H
#define PSP_ICE_INPUT_H

#include <stdint.h>

uint16_t ice_buttons_from_psp(uint32_t buttons);
void ice_input_payload(uint8_t *out, uint16_t buttons, uint8_t analog_x, uint8_t analog_y);

#endif
