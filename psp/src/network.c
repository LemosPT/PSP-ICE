#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspkernel.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "network.h"

static int net_initialized;

int ice_net_init(void) {
    int ret;
    if (net_initialized) return 0;

    ret = sceNetInit(128 * 1024, 42, 0, 42, 0);
    if (ret < 0) return ret;
    ret = sceNetInetInit();
    if (ret < 0) return ret;
    ret = sceNetApctlInit(0x1800, 48);
    if (ret < 0) return ret;
    net_initialized = 1;
    return 0;
}

int ice_wifi_connect(int profile) {
    int ret;
    int state;
    ret = sceNetApctlConnect(profile);
    sceNetApctlGetState(&state);
    sceKernelDelayThread(1000000);
    if (ret < 0) return ret;

    return 0;
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
