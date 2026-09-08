#ifndef PSP_ICE_PROTOCOL_H
#define PSP_ICE_PROTOCOL_H

#include <stdint.h>

#define PSP_ICE_MAGIC "ICE1"
#define PSP_ICE_VERSION 1

#define PSP_ICE_CHANNEL_CONTROL 0
#define PSP_ICE_CHANNEL_VIDEO 1
#define PSP_ICE_CHANNEL_INPUT 2
#define PSP_ICE_CHANNEL_AUDIO 3

#define PSP_ICE_MSG_HELLO 1
#define PSP_ICE_MSG_PING 2
#define PSP_ICE_MSG_PONG 3
#define PSP_ICE_MSG_INPUT_STATE 10
#define PSP_ICE_MSG_VIDEO_FRAME 20

/* Network byte order. Total header size: 14 bytes. */
typedef struct {
    char magic[4];
    uint8_t version;
    uint8_t channel;
    uint8_t type;
    uint8_t flags;
    uint32_t sequence;
    uint16_t payload_length;
} psp_ice_header_t;

/* INPUT_STATE payload: buttons (uint16, network order), analog X/Y. */
typedef struct {
    uint16_t buttons;
    uint8_t analog_x;
    uint8_t analog_y;
} psp_ice_input_state_t;

#define PSP_ICE_BTN_SELECT (1u << 0)
#define PSP_ICE_BTN_START  (1u << 1)
#define PSP_ICE_BTN_UP     (1u << 2)
#define PSP_ICE_BTN_RIGHT  (1u << 3)
#define PSP_ICE_BTN_DOWN   (1u << 4)
#define PSP_ICE_BTN_LEFT   (1u << 5)
#define PSP_ICE_BTN_L      (1u << 6)
#define PSP_ICE_BTN_R      (1u << 7)
#define PSP_ICE_BTN_TRIANGLE (1u << 8)
#define PSP_ICE_BTN_CIRCLE   (1u << 9)
#define PSP_ICE_BTN_CROSS    (1u << 10)
#define PSP_ICE_BTN_SQUARE   (1u << 11)
#define PSP_ICE_BTN_HOME     (1u << 12)
#define PSP_ICE_BTN_HOLD     (1u << 13)

#endif
