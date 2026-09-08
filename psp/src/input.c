#include <pspctrl.h>
#include <stdint.h>
#include <string.h>

#include "input.h"
#include "protocol.h"

uint16_t ice_buttons_from_psp(uint32_t b) {
    uint16_t out = 0;

    if (b & PSP_CTRL_SELECT)   out |= PSP_ICE_BTN_SELECT;
    if (b & PSP_CTRL_START)    out |= PSP_ICE_BTN_START;
    if (b & PSP_CTRL_UP)       out |= PSP_ICE_BTN_UP;
    if (b & PSP_CTRL_RIGHT)    out |= PSP_ICE_BTN_RIGHT;
    if (b & PSP_CTRL_DOWN)     out |= PSP_ICE_BTN_DOWN;
    if (b & PSP_CTRL_LEFT)     out |= PSP_ICE_BTN_LEFT;
    if (b & PSP_CTRL_LTRIGGER) out |= PSP_ICE_BTN_L;
    if (b & PSP_CTRL_RTRIGGER) out |= PSP_ICE_BTN_R;
    if (b & PSP_CTRL_TRIANGLE) out |= PSP_ICE_BTN_TRIANGLE;
    if (b & PSP_CTRL_CIRCLE)   out |= PSP_ICE_BTN_CIRCLE;
    if (b & PSP_CTRL_CROSS)    out |= PSP_ICE_BTN_CROSS;
    if (b & PSP_CTRL_SQUARE)   out |= PSP_ICE_BTN_SQUARE;
    if (b & PSP_CTRL_HOME)     out |= PSP_ICE_BTN_HOME;
    if (b & PSP_CTRL_HOLD)     out |= PSP_ICE_BTN_HOLD;

    return out;
}

void ice_input_payload(uint8_t *out, uint16_t buttons, uint8_t analog_x, uint8_t analog_y) {
    /* INPUT_STATE payload is 4 bytes: uint16 buttons in network order, X, Y. */
    out[0] = (uint8_t)(buttons >> 8);
    out[1] = (uint8_t)(buttons & 0xff);
    out[2] = analog_x;
    out[3] = analog_y;
}
