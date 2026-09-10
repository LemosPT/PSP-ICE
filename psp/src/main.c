#include <pspctrl.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <psputility_sysparam.h>
#include <stdio.h>
#include <string.h>

#include "input.h"
#include "network.h"
#include "protocol.h"

PSP_MODULE_INFO("PSP-ICE", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);

#define GATEWAY_PORT 39000
#define PACKET_SIZE 64
#define INPUT_INTERVAL_US 33333

/* Change this to the PC running the PSP-ICE gateway. */
static const char *gateway_host = "192.168.1.143";

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

int main(void) {
    int sock;
    int ret;
    uint32_t sequence = 0;
    uint8_t packet[PACKET_SIZE];
    uint8_t payload[4];
    SceCtrlData pad;

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

    pspDebugScreenPrintf("Sending HELLO...\n");
    ret = send_hello(sock, &sequence);
    if (ret < 0) {
        pspDebugScreenPrintf("HELLO send FAILED: 0x%08X\n", (unsigned int)ret);
    } else {
        pspDebugScreenPrintf("HELLO sent, seq=%lu\n", (unsigned long)sequence);
    }

    pspDebugScreenPrintf("\nMove the PSP controls.\n");
    pspDebugScreenPrintf("Gateway should print INPUT packets.\n\n");

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    while (1) {
        int len;
        uint16_t buttons;

        sceCtrlReadBufferPositive(&pad, 1);
        buttons = ice_buttons_from_psp(pad.Buttons);
        ice_input_payload(payload, buttons, pad.Lx, pad.Ly);

        len = build_packet(packet, PSP_ICE_CHANNEL_INPUT,
                           PSP_ICE_MSG_INPUT_STATE, ++sequence,
                           payload, sizeof(payload));
        if (len > 0) {
            ice_net_send(sock, gateway_host, GATEWAY_PORT, packet, len);
        }

        pspDebugScreenSetXY(0, 8);
        pspDebugScreenPrintf("seq: %lu     \n", (unsigned long)sequence);
        pspDebugScreenPrintf("buttons: 0x%04X\n", buttons);
        pspDebugScreenPrintf("analog:  %3d,%3d\n", pad.Lx, pad.Ly);
        pspDebugScreenPrintf("                       \n");

        sceKernelDelayThread(INPUT_INTERVAL_US);
    }

    return 0;
}
