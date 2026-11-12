#include <pspctrl.h>
#include <pspdebug.h>
#include <pspdisplay.h>
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


static void draw_dashboard(const ice_audio_stats_t *stats,
                           unsigned int lost, unsigned int expected,
                           unsigned int duplicates, unsigned int reordered) {
    char value[32];
    unsigned int loss_tenths = expected ? (lost * 1000U) / expected : 0;

    ice_gfx_begin();

    /* Header */
    ice_gfx_rect(0, 0, 480, 36, GU_COLOR(16, 24, 35, 255));
    ice_gfx_rect(0, 35, 480, 2, GU_COLOR(0, 153, 255, 255));
    ice_gfx_text(18, 13, "PSP-ICE", 2, GU_COLOR(235, 243, 250, 255));
    ice_gfx_text(350, 15, "WIFI ONLINE", 1, GU_COLOR(80, 205, 150, 255));

    /* Navigation rail */
    ice_gfx_rect(14, 50, 105, 207, GU_COLOR(17, 25, 36, 255));
    ice_gfx_rect(14, 60, 3, 25, GU_COLOR(0, 153, 255, 255));
    ice_gfx_text(28, 69, "HOME", 1, GU_COLOR(90, 190, 255, 255));
    ice_gfx_text(28, 112, "MUSIC", 1, GU_COLOR(180, 192, 205, 255));
    ice_gfx_text(28, 155, "PHONE", 1, GU_COLOR(180, 192, 205, 255));
    ice_gfx_text(28, 198, "SETTINGS", 1, GU_COLOR(180, 192, 205, 255));
    ice_gfx_rect(26, 94, 80, 1, GU_COLOR(42, 54, 69, 255));
    ice_gfx_rect(26, 137, 80, 1, GU_COLOR(42, 54, 69, 255));
    ice_gfx_rect(26, 180, 80, 1, GU_COLOR(42, 54, 69, 255));

    /* Main now-playing panel */
    ice_gfx_rect(132, 50, 334, 126, GU_COLOR(17, 25, 36, 255));
    ice_gfx_text(150, 65, "NOW PLAYING", 1, GU_COLOR(115, 137, 158, 255));
    ice_gfx_rect(151, 91, 72, 72, GU_COLOR(27, 43, 61, 255));
    ice_gfx_rect(162, 102, 50, 50, GU_COLOR(0, 105, 180, 255));
    ice_gfx_text(171, 122, "ICE", 1, GU_COLOR(255, 255, 255, 255));
    ice_gfx_text(238, 105, "NO TRACK", 2, GU_COLOR(235, 243, 250, 255));
    ice_gfx_text(239, 133, "WAITING FOR IPHONE", 1,
                 GU_COLOR(125, 145, 165, 255));

    /* Audio and network cards */
    ice_gfx_rect(132, 188, 160, 69, GU_COLOR(17, 25, 36, 255));
    ice_gfx_rect(300, 188, 166, 69, GU_COLOR(17, 25, 36, 255));
    ice_gfx_text(146, 200, "AUDIO LINK", 1, GU_COLOR(115, 137, 158, 255));
    ice_gfx_text(314, 200, "PACKET HEALTH", 1, GU_COLOR(115, 137, 158, 255));

    snprintf(value, sizeof(value), "RX %u  PLAY %u",
             stats->packets_received, stats->packets_played);
    ice_gfx_text(146, 219, value, 1, GU_COLOR(220, 230, 240, 255));

    if (expected) {
        snprintf(value, sizeof(value), "LOSS %u.%u%%",
                 loss_tenths / 10, loss_tenths % 10);
    } else {
        snprintf(value, sizeof(value), "LOSS --.-%%");
    }
    ice_gfx_text(314, 219, value, 1,
                 lost ? GU_COLOR(255, 190, 90, 255)
                      : GU_COLOR(100, 210, 155, 255));

    snprintf(value, sizeof(value), "DROP %u  ERR %u",
             stats->packets_dropped, stats->playback_errors);
    ice_gfx_text(146, 237, value, 1, GU_COLOR(125, 145, 165, 255));

    snprintf(value, sizeof(value), "DUP %u  OOO %u",
             duplicates, reordered);
    ice_gfx_text(314, 237, value, 1, GU_COLOR(125, 145, 165, 255));

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
                       audio_out_of_order);
        sceKernelDelayThread(INPUT_INTERVAL_US);
    }

    return 0;
}
