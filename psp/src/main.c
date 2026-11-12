#include <pspctrl.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspkernel.h>
#include <psputility_sysparam.h>
#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "input.h"
#include "graphics.h"
#include "network.h"
#include "protocol.h"

PSP_MODULE_INFO("PSP-ICE", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);

#define GATEWAY_PORT 39000
#define PACKET_SIZE 64
#define INPUT_INTERVAL_US 33333

/* UDP broadcast lets the gateway on the DHCP-assigned local network receive
 * packets without requiring its address to be hard-coded on the PSP. */
static const char *gateway_host = "255.255.255.255";

static int exit_callback(int arg1, int arg2, void *common) {
    sceKernelExitGame();
    return 0;
}

static int callback_thread(SceSize args, void *argp) {
    int callback;

    callback = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    if (callback >= 0) sceKernelRegisterExitCallback(callback);
    sceKernelSleepThreadCB();
    return 0;
}

static void setup_callbacks(void) {
    SceUID thread;

    thread = sceKernelCreateThread("update_thread", callback_thread,
                                   0x11, 0xFA0, PSP_THREAD_ATTR_USER, NULL);
    if (thread >= 0) sceKernelStartThread(thread, 0, NULL);
}

static int build_packet(uint8_t *out, uint8_t channel, uint8_t type,
                        uint32_t sequence, const uint8_t *payload,
                        uint16_t payload_len) {
    if (payload_len > PACKET_SIZE - 14) return -1;

    memcpy(out, PSP_ICE_MAGIC, 4);
    out[4] = PSP_ICE_VERSION;
    out[5] = channel;
    out[6] = type;
    out[7] = 0;
    out[8] = (uint8_t)(sequence >> 24);
    out[9] = (uint8_t)(sequence >> 16);
    out[10] = (uint8_t)(sequence >> 8);
    out[11] = (uint8_t)sequence;
    out[12] = (uint8_t)(payload_len >> 8);
    out[13] = (uint8_t)payload_len;
    if (payload_len) memcpy(out + 14, payload, payload_len);
    return 14 + payload_len;
}

static int send_hello(int sock, uint32_t *sequence) {
    uint8_t packet[PACKET_SIZE];
    const uint8_t payload[] = "PSP-TEST/1";
    int len = build_packet(packet, PSP_ICE_CHANNEL_CONTROL, PSP_ICE_MSG_HELLO,
                            ++(*sequence), payload, sizeof(payload) - 1);
    if (len < 0) return len;
    return ice_net_send(sock, gateway_host, GATEWAY_PORT, packet, len);
}


#define UI_COLOR_BG       GU_COLOR(7, 12, 19, 255)
#define UI_COLOR_PANEL    GU_COLOR(18, 28, 41, 255)
#define UI_COLOR_PANEL2   GU_COLOR(25, 38, 54, 255)
#define UI_COLOR_BLUE     GU_COLOR(0, 145, 235, 255)
#define UI_COLOR_WHITE    GU_COLOR(238, 244, 250, 255)
#define UI_COLOR_MUTED    GU_COLOR(132, 151, 171, 255)
#define UI_COLOR_GREEN    GU_COLOR(78, 207, 151, 255)

enum {
    UI_HOME = 0,
    UI_MUSIC,
    UI_PHONE,
    UI_MAPS,
    UI_APPS,
    UI_SETTINGS
};

static const char *ui_names[] = {
    "MUSIC", "PHONE", "MAPS", "APPS", "SETTINGS", "CONNECT"
};

static const char *ui_symbols[] = {
    "M", "P", "N", "A", "S", "C"
};

static void draw_status_bar(void) {
    ice_gfx_rect(0, 0, 480, 27, GU_COLOR(12, 19, 28, 255));
    ice_gfx_text(13, 10, "PSP-ICE", 1, UI_COLOR_WHITE);
    ice_gfx_text(365, 10, "WIFI", 1, UI_COLOR_GREEN);
    ice_gfx_rect(405, 9, 2, 9, UI_COLOR_GREEN);
    ice_gfx_rect(410, 6, 2, 12, UI_COLOR_GREEN);
    ice_gfx_rect(415, 3, 2, 15, UI_COLOR_GREEN);
    ice_gfx_text(431, 10, "BT", 1, UI_COLOR_MUTED);
    ice_gfx_rect(0, 26, 480, 1, GU_COLOR(35, 50, 66, 255));
}

static void draw_media_strip(void) {
    ice_gfx_rect(0, 226, 480, 46, GU_COLOR(13, 21, 31, 255));
    ice_gfx_rect(0, 226, 480, 1, GU_COLOR(35, 50, 66, 255));
    ice_gfx_rect(12, 233, 31, 31, GU_COLOR(0, 93, 160, 255));
    ice_gfx_text(22, 245, "ICE", 1, UI_COLOR_WHITE);
    ice_gfx_text(53, 237, "NOTHING PLAYING", 1, UI_COLOR_WHITE);
    ice_gfx_text(53, 252, "CONNECT YOUR IPHONE", 1, UI_COLOR_MUTED);
    ice_gfx_text(386, 245, "<<", 1, UI_COLOR_MUTED);
    ice_gfx_text(416, 245, ">", 1, UI_COLOR_WHITE);
    ice_gfx_text(445, 245, ">>", 1, UI_COLOR_MUTED);
}

static void draw_home(int selected) {
    int i;
    int x, y;
    const int tile_w = 126;
    const int tile_h = 79;
    const int gap_x = 9;
    const int gap_y = 8;

    draw_status_bar();

    ice_gfx_text(15, 42, "GOOD EVENING", 1, UI_COLOR_MUTED);
    ice_gfx_text(15, 57, "YOUR APPS", 1, UI_COLOR_WHITE);

    for (i = 0; i < 6; i++) {
        int col = i % 3;
        int row = i / 3;
        x = 15 + col * (tile_w + gap_x);
        y = 76 + row * (tile_h + gap_y);

        ice_gfx_rect(x, y, tile_w, tile_h,
                     i == selected ? GU_COLOR(23, 54, 80, 255) : UI_COLOR_PANEL);
        if (i == selected) {
            ice_gfx_rect(x, y, tile_w, 3, UI_COLOR_BLUE);
            ice_gfx_rect(x, y, 3, tile_h, UI_COLOR_BLUE);
        } else {
            ice_gfx_rect(x, y, tile_w, 1, GU_COLOR(34, 49, 65, 255));
        }

        ice_gfx_rect(x + 10, y + 12, 34, 34,
                     i == selected ? UI_COLOR_BLUE : UI_COLOR_PANEL2);
        ice_gfx_text(x + 22, y + 24, ui_symbols[i], 1, UI_COLOR_WHITE);
        ice_gfx_text(x + 53, y + 25, ui_names[i], 1, UI_COLOR_WHITE);
        ice_gfx_text(x + 53, y + 42, i == 0 ? "AUDIO" :
                     i == 1 ? "CALLS" :
                     i == 2 ? "NAVIGATION" :
                     i == 3 ? "LIBRARY" :
                     i == 4 ? "PREFERENCES" : "IPHONE LINK",
                     1, UI_COLOR_MUTED);
    }

    draw_media_strip();
}

static void draw_app_page(int page) {
    const char *name = page > 0 && page <= 6 ? ui_names[page - 1] : "APP";
    draw_status_bar();

    ice_gfx_rect(14, 43, 452, 172, UI_COLOR_PANEL);
    ice_gfx_rect(14, 43, 3, 172, UI_COLOR_BLUE);
    ice_gfx_text(30, 58, name, 2, UI_COLOR_WHITE);
    ice_gfx_rect(30, 83, 420, 1, GU_COLOR(42, 58, 76, 255));

    if (page == UI_MUSIC) {
        ice_gfx_rect(39, 100, 78, 78, GU_COLOR(0, 93, 160, 255));
        ice_gfx_text(65, 132, "ICE", 2, UI_COLOR_WHITE);
        ice_gfx_text(137, 112, "NOTHING PLAYING", 2, UI_COLOR_WHITE);
        ice_gfx_text(138, 143, "CONNECT YOUR IPHONE", 1, UI_COLOR_MUTED);
        ice_gfx_text(138, 170, "PLAYBACK CONTROLS WILL APPEAR HERE", 1, UI_COLOR_MUTED);
    } else {
        ice_gfx_text(34, 112, "THIS SECTION IS NOT READY YET", 2, UI_COLOR_WHITE);
        ice_gfx_text(35, 145, "WE WILL BUILD IT NEXT", 1, UI_COLOR_MUTED);
    }

    ice_gfx_text(30, 195, "O  BACK TO HOME", 1, UI_COLOR_MUTED);
    draw_media_strip();
}

static void draw_dashboard(const ice_audio_stats_t *stats,
                           unsigned int lost, unsigned int expected,
                           unsigned int duplicates, unsigned int reordered,
                           int page, int selected) {
    char value[32];
    unsigned int loss_tenths = expected ? (lost * 1000U) / expected : 0;
    (void)stats;
    (void)duplicates;
    (void)reordered;

    if (page != UI_HOME) {
        draw_app_page(page);
        ice_gfx_end();
        return;
    }

    draw_home(selected);

    /* Small live link-health line, kept subtle like an OEM status readout. */
    if (expected) {
        snprintf(value, sizeof(value), "LINK LOSS %u.%u%%",
                 loss_tenths / 10, loss_tenths % 10);
        ice_gfx_text(330, 211, value, 1,
                     lost ? GU_COLOR(255, 190, 90, 255) : UI_COLOR_GREEN);
    }

    ice_gfx_end();
}

int main(void) {
    int sock;
    int ret;
    uint32_t sequence = 0;
    uint32_t last_audio_sequence = 0;
    unsigned int audio_packets_lost = 0;
    unsigned int audio_packets_expected = 0;
    unsigned int audio_duplicates = 0;
    unsigned int audio_out_of_order = 0;
    int audio_sequence_initialized = 0;
    uint8_t packet[PACKET_SIZE];
    uint8_t audio_packet[1200];
    uint8_t payload[4];
    SceCtrlData pad;
    ice_audio_stats_t audio_stats;
    int ui_page = UI_HOME;
    int ui_selected = 0;
    uint32_t previous_buttons = 0;

    setup_callbacks();
    pspDebugScreenInit();
    pspDebugScreenPrintf("PSP-ICE PSP input test\n\n");
    pspDebugScreenPrintf("Gateway: %s:%d\n", gateway_host, GATEWAY_PORT);
    pspDebugScreenPrintf("Starting network...\n");


    ret = ice_net_init();
    if (ret < 0) {
        pspDebugScreenPrintf("Network init FAILED: 0x%08X\n", (unsigned int)ret);
        pspDebugScreenPrintf("Failed at network stage %d\n", ice_net_init_stage());
        pspDebugScreenPrintf("1=COMMON 2=INET 3=NET 4=INET-INIT 5=APCTL\n");
        sceKernelSleepThread();
        return 1;
    }

    pspDebugScreenPrintf("Network started. Connecting to Wi-Fi...\n");

    ret = ice_wifi_connect(1);
    if (ret < 0) {
        pspDebugScreenPrintf("Wi-Fi connection FAILED: 0x%08X\n", (unsigned int)ret);
        ice_net_shutdown();
        sceKernelSleepThread();
        return 1;
    }

    pspDebugScreenInit();
    sock = ice_net_socket();
    if (sock < 0) {
        pspDebugScreenPrintf("Socket creation FAILED: 0x%08X\n", (unsigned int)sock);
        ice_net_shutdown();
        sceKernelSleepThread();
        return 1;
    }

    ret = ice_net_bind(sock, GATEWAY_PORT);
    if (ret < 0) {
        pspDebugScreenPrintf("UDP bind FAILED: 0x%08X\n", (unsigned int)ret);
        ice_net_shutdown();
        sceKernelSleepThread();
        return 1;
    }

    ret = ice_audio_init();
    if (ret < 0) {
        pspDebugScreenPrintf("Audio init FAILED: 0x%08X\n", (unsigned int)ret);
        ice_net_shutdown();
        sceKernelSleepThread();
        return 1;
    }

    pspDebugScreenPrintf("Sending HELLO...\n");
    ret = send_hello(sock, &sequence);
    if (ret < 0) {
        pspDebugScreenPrintf("HELLO send FAILED: 0x%08X\n", (unsigned int)ret);
    } else {
        pspDebugScreenPrintf("HELLO sent, seq=%lu\n", (unsigned long)sequence);
    }

    pspDebugScreenPrintf("\nAudio receiver ready on UDP %d.\n", GATEWAY_PORT);
    pspDebugScreenPrintf("Move controls; stream PCM to PSP.\n\n");

    ice_gfx_init();

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    while (1) {
        int len;
        uint16_t buttons;
        int received;

        /* Drain queued audio datagrams before the 30 Hz input update.
         * Audio packets arrive about every 5.8 ms, so reading only one
         * packet per controller tick would lose most of the stream. */
        {
            int packets_read = 0;
            do {
                received = ice_net_receive(sock, audio_packet,
                                           sizeof(audio_packet), 0);
                if (received >= 14 &&
                    memcmp(audio_packet, PSP_ICE_MAGIC, 4) == 0 &&
                    audio_packet[4] == PSP_ICE_VERSION &&
                    audio_packet[5] == PSP_ICE_CHANNEL_AUDIO &&
                    audio_packet[6] == PSP_ICE_MSG_AUDIO_DATA) {
                    unsigned int payload_len =
                        ((unsigned int)audio_packet[12] << 8) | audio_packet[13];
                    if (payload_len == 1024 && received == 14 + payload_len) {
                        uint32_t received_sequence =
                            ((uint32_t)audio_packet[8] << 24) |
                            ((uint32_t)audio_packet[9] << 16) |
                            ((uint32_t)audio_packet[10] << 8) |
                            (uint32_t)audio_packet[11];

                        /* Track audio packet sequence numbers. */
                        if (!audio_sequence_initialized) {
                            last_audio_sequence = received_sequence;
                            audio_sequence_initialized = 1;
                            audio_packets_expected = 1;
                        } else {
                            uint32_t sequence_delta =
                                received_sequence - last_audio_sequence;

                            if (sequence_delta == 0) {
                                audio_duplicates++;
                            } else if (sequence_delta < 0x80000000) {
                                audio_packets_expected += sequence_delta;

                                if (sequence_delta > 1) {
                                    audio_packets_lost += sequence_delta - 1;
                                }

                                last_audio_sequence = received_sequence;
                            } else {
                                audio_out_of_order++;
                            }
                        }

                        ice_audio_queue(audio_packet + 14, payload_len);
                    }
                }
                packets_read++;
            } while (received > 0 && packets_read < 8);
        }

        sceCtrlReadBufferPositive(&pad, 1);
        buttons = ice_buttons_from_psp(pad.Buttons);

        /* Navigate the local CarPlay-style interface with the PSP D-pad. */
        {
            uint32_t pressed = pad.Buttons & ~previous_buttons;
            if (ui_page == UI_HOME) {
                if (pressed & PSP_CTRL_LEFT) {
                    if (ui_selected % 3 > 0) ui_selected--;
                } else if (pressed & PSP_CTRL_RIGHT) {
                    if (ui_selected % 3 < 2) ui_selected++;
                } else if (pressed & PSP_CTRL_UP) {
                    if (ui_selected >= 3) ui_selected -= 3;
                } else if (pressed & PSP_CTRL_DOWN) {
                    if (ui_selected < 3) ui_selected += 3;
                } else if (pressed & PSP_CTRL_CROSS) {
                    ui_page = ui_selected + 1;
                }
            } else if (pressed & PSP_CTRL_CIRCLE) {
                ui_page = UI_HOME;
            }
            previous_buttons = pad.Buttons;
        }
        ice_input_payload(payload, buttons, pad.Lx, pad.Ly);

        len = build_packet(packet, PSP_ICE_CHANNEL_INPUT,
                           PSP_ICE_MSG_INPUT_STATE, ++sequence,
                           payload, sizeof(payload));
        if (len > 0) {
            ice_net_send(sock, gateway_host, GATEWAY_PORT, packet, len);
        }
        
        ice_audio_get_stats(&audio_stats);
        draw_dashboard(&audio_stats, audio_packets_lost,
                       audio_packets_expected, audio_duplicates,
                       audio_out_of_order, ui_page, ui_selected);
        sceKernelDelayThread(INPUT_INTERVAL_US);
    }

    return 0;
}
