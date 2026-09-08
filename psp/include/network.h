#ifndef PSP_ICE_NETWORK_H
#define PSP_ICE_NETWORK_H

#include <stdint.h>

int ice_net_init(void);
int ice_net_init_stage(void);
int ice_wifi_connect(int profile);
void ice_net_shutdown(void);
int ice_net_socket(void);
int ice_net_send(int sock, const char *host, uint16_t port,
                const void *data, uint16_t length);
int ice_net_receive(int sock, void *data, uint16_t capacity,
                    uint32_t timeout_us);

#endif
