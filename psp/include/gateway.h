#ifndef PSP_ICE_GATEWAY_H
#define PSP_ICE_GATEWAY_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

void gateway_init(void);
static void gateway_reset_reassembly(void);
static bool gateway_init_framebuffer(gateway_framebuffer_t *fb, uint8_t *front, uint8_t *back);
static bool gateway_is_480x272_rgb565_frame(const video_frame_header_t *hdr);
static bool gateway_packet_has_valid_payload_size(size_t payload_len, uint16_t fragment_id, uint16_t fragment_count);
static inline uint32_t gateway_fragment_offset(uint16_t fragment_id, uint16_t fragment_count);
//static gateway_video_status_t gateway_accept_new_frame(uint16_t frame_id, uint16_t fragment_count, uint32_t frame_size);
static bool gateway_store_fragment(const video_frame_header_t *hdr, const uint8_t *payload, size_t payload_len);
static bool gateway_frame_is_complete(void);
static bool gateway_submit_completed_frame(gateway_framebuffer_t *fb, uint8_t *frame_data);
static void gateway_swap_framebuffers(gateway_framebuffer_t *fb);
//static gateway_video_status_t gateway_handle_udp_video_packet(gateway_framebuffer_t *fb,
//                                                             const uint8_t *packet,
//                                                             size_t packet_len);
int gateway_example_usage(void);


#endif