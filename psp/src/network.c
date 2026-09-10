#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspkernel.h>
#include <psputility_netmodules.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <psputility_netconf.h>

#include "network.h"

static int net_initialized;
static int net_stage_error;

int ice_net_init(void) {
    int ret;

    net_stage_error = 0;

    if (net_initialized) return 0;

    /* Load the PSP networking modules before using any sceNet* API. */
    ret = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
    if (ret < 0) {
        net_stage_error = 1;
        return ret;
    }

    ret = sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
    if (ret < 0) {
        net_stage_error = 2;
        return ret;
    }

    /* These are the same initialization values used by the PSPSDK
       networking sample. */
    ret = sceNetInit(128 * 1024, 42, 4 * 1024, 42, 4 * 1024);
    if (ret < 0) {
        net_stage_error = 3;
        return ret;
    }

    ret = sceNetInetInit();
    if (ret < 0) {
        net_stage_error = 4;
        sceNetTerm();
        return ret;
    }

    ret = sceNetApctlInit(0x8000, 48);
    if (ret < 0) {
        net_stage_error = 5;
        sceNetInetTerm();
        sceNetTerm();
        return ret;
    }

    net_initialized = 1;
    return 0;
}

int ice_net_init_stage(void) {
    return net_stage_error;
}


int ice_wifi_connect(int profile) {
    int profile = 0; // Default to the first profile

    SceUtilityNetconfData netconf;
    memset(&netconf, 0, sizeof(netconf));
    netconf.size = sizeof(netconf);

    // Start the PSP network configuration dialog
    sceUtilityNetconfInitStart(&netconf);

    While (1) {
        int status = sceUtilityNetconfGetStatus();
        if (status == 2) break; // finished
        if (status < 0) return status; // error
        sceKernelDelayThread(10000);
    }

    // Then shutdown the dialog
    sceUtilityNetconfShutdownStart();

    // After the user selected a profile, use that profile index
    // profile = selected_profile_index;

    ret = sceNetApctlConnect(profile);
    if (ret < 0) return ret;

    /* Wait for the connection to progress through the APCTL states. */
    while (attempts < 200) { /* 200 x 50 ms = 10 seconds */
        ret = sceNetApctlGetState(&state);
        if (ret < 0) return ret;

        if (state == PSP_NET_APCTL_STATE_GOT_IP) {
            return 0;
        }

        if (state == PSP_NET_APCTL_STATE_DISCONNECTED) {
            return -1;
        }

        sceKernelDelayThread(50 * 1000);
        attempts++;
    }

    return -1;
}

void ice_net_shutdown(void) {
    if (!net_initialized) return;

    sceNetApctlTerm();
    sceNetInetTerm();
    sceNetTerm();
    net_initialized = 0;
}

int ice_net_socket(void) {
    return sceNetInetSocket(AF_INET, SOCK_DGRAM, 0);
}

int ice_net_send(int sock, const char *host, uint16_t port,
                 const void *data, uint16_t length) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = sceNetInetInetAddr(host);
    if (addr.sin_addr.s_addr == (uint32_t)-1) return -1;

    return sceNetInetSendto(sock, data, length, 0,
                            (struct sockaddr *)&addr, sizeof(addr));
}

int ice_net_receive(int sock, void *data, uint16_t capacity,
                    uint32_t timeout_us) {
    struct SceNetInetTimeval tv;
    fd_set readfds;
    int ret;

    tv.tv_sec = timeout_us / 1000000;
    tv.tv_usec = timeout_us % 1000000;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    ret = sceNetInetSelect(sock + 1, &readfds, NULL, NULL, &tv);
    if (ret <= 0) return ret;
    return sceNetInetRecv(sock, data, capacity, 0);
}

//MAKE THE PSP OPEN THE MENU OF WI-FI'S INSTEAD OF OPENING A RANDOM PROFILE AND PRAYING

//int profile = 0;

//SceUtilityNetconfData netconf;
//memset(&netconf, 0, sizeof(netconf));
//netconf.size = sizeof(netconf);

//// Start the PSP network configuration dialog
//sceUtilityNetconfInitStart(&netconf);

//while (1) {
//    int status = sceUtilityNetconfGetStatus();
//    if (status == 2) break; // finished
//    if (status < 0) break; // error
//    sceKernelDelayThread(10000);
//}

//// Then shutdown the dialog
//sceUtilityNetconfShutdownStart();

//// After the user selected a profile, use that profile index
//// e.g. profile = selected_profile_index;
//ret = sceNetApctlConnect(profile);
