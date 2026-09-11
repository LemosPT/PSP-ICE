#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UDP_PORT 39000u
#define VIDEO_FRAME 20u

#define FRAME_WIDTH 480u
#define FRAME_HEIGHT 272u
#define BYTES_PER_PIXEL 2u
#define RAW_FRAME_SIZE ((FRAME_WIDTH * FRAME_HEIGHT) * BYTES_PER_PIXEL)
#define VIDEO_HEADER_SIZE 14u
#define MAX_PIXEL_DATA_PER_UDP 59986u
#define COMPLETE_FRAME_FRAGMENTS 5u
#define MAX_UDP_PAYLOAD_SIZE (VIDEO_HEADER_SIZE + MAX_PIXEL_DATA_PER_UDP)

#define VIDEO_MAGIC 0x5649444Fu /* 'VIDO' */

#pragma pack(push, 1)
typedef struct {
    uint8_t type;
    uint8_t version;
    uint16_t frame_id;
    uint16_t fragment_id;
    uint16_t fragment_count;
    uint32_t frame_size;
    uint16_t flags;
} video_frame_header_t;
#pragma pack(pop)

#define VIDEO_HEADER_FIELD_BYTES (sizeof(video_frame_header_t))

typedef enum {
    GATEWAY_VIDEO_OK = 0,
    GATEWAY_VIDEO_BAD_TYPE = 1,
    GATEWAY_VIDEO_BAD_LENGTH = 2,
    GATEWAY_VIDEO_BAD_FRAME_ID = 3,
    GATEWAY_VIDEO_BAD_SIZE = 4,
    GATEWAY_VIDEO_DUPLICATE = 5,
    GATEWAY_VIDEO_INCOMPLETE = 6,
    GATEWAY_VIDEO_OBSOLETE = 7,
    GATEWAY_VIDEO_READY = 8
} gateway_video_status_t;

typedef struct {
    bool active;
    uint16_t frame_id;
    uint16_t fragment_count;
    uint32_t frame_size;
    uint32_t received_mask;
    uint16_t received_fragments;
    uint8_t *buffer;
} gateway_frame_reassembly_t;

typedef struct {
    uint8_t *front;
    uint8_t *back;
    volatile bool back_ready;
    uint16_t latest_frame_id;
} gateway_framebuffer_t;

static gateway_frame_reassembly_t g_reassembly = {0};
static gateway_framebuffer_t g_framebuffer = {0};

static void gateway_reset_reassembly(void) {
    if (g_reassembly.buffer != NULL) {
        memset(g_reassembly.buffer, 0, RAW_FRAME_SIZE);
    }
    g_reassembly.active = false;
    g_reassembly.frame_id = 0;
    g_reassembly.fragment_count = 0;
    g_reassembly.frame_size = 0;
    g_reassembly.received_mask = 0u;
    g_reassembly.received_fragments = 0u;
}

static bool gateway_init_framebuffer(gateway_framebuffer_t *fb, uint8_t *front, uint8_t *back) {
    if (fb == NULL || front == NULL || back == NULL) {
        return false;
    }
    fb->front = front;
    fb->back = back;
    fb->back_ready = false;
    fb->latest_frame_id = 0u;
    memset(front, 0, RAW_FRAME_SIZE);
    memset(back, 0, RAW_FRAME_SIZE);
    return true;
}

static bool gateway_is_480x272_rgb565_frame(const video_frame_header_t *hdr) {
    if (hdr == NULL) {
        return false;
    }
    return (hdr->fragment_count == COMPLETE_FRAME_FRAGMENTS &&
            hdr->frame_size == RAW_FRAME_SIZE &&
            hdr->type == VIDEO_FRAME &&
            hdr->version == 1u);
}

static bool gateway_packet_has_valid_payload_size(size_t payload_len, uint16_t fragment_id, uint16_t fragment_count) {
    if (payload_len < VIDEO_HEADER_FIELD_BYTES) {
        return false;
    }
    if (fragment_count == 0u || fragment_id >= fragment_count) {
        return false;
    }
    if ((payload_len - VIDEO_HEADER_FIELD_BYTES) > MAX_PIXEL_DATA_PER_UDP) {
        return false;
    }
    return true;
}

static inline uint32_t gateway_fragment_offset(uint16_t fragment_id, uint16_t fragment_count) {
    uint32_t chunk_size = MAX_PIXEL_DATA_PER_UDP;
    (void)fragment_count;
    return (uint32_t)fragment_id * chunk_size;
}

static gateway_video_status_t gateway_accept_new_frame(uint16_t frame_id, uint16_t fragment_count, uint32_t frame_size) {
    if (frame_id == 0u) {
        return GATEWAY_VIDEO_BAD_FRAME_ID;
    }
    if (fragment_count != COMPLETE_FRAME_FRAGMENTS) {
        return GATEWAY_VIDEO_BAD_LENGTH;
    }
    if (frame_size != RAW_FRAME_SIZE) {
        return GATEWAY_VIDEO_BAD_SIZE;
    }
    if (g_reassembly.active && g_reassembly.frame_id != frame_id) {
        /* Drop the old incomplete frame when a newer one starts. */
        gateway_reset_reassembly();
    }
    if (!g_reassembly.active) {
        g_reassembly.active = true;
        g_reassembly.frame_id = frame_id;
        g_reassembly.fragment_count = fragment_count;
        g_reassembly.frame_size = frame_size;
        g_reassembly.received_mask = 0u;
        g_reassembly.received_fragments = 0u;
        if (g_reassembly.buffer == NULL) {
            g_reassembly.buffer = (uint8_t *)calloc(1u, RAW_FRAME_SIZE);
            if (g_reassembly.buffer == NULL) {
                gateway_reset_reassembly();
                return GATEWAY_VIDEO_BAD_LENGTH;
            }
        }
    }
    return GATEWAY_VIDEO_OK;
}

static bool gateway_store_fragment(const video_frame_header_t *hdr, const uint8_t *payload, size_t payload_len) {
    if (hdr == NULL || payload == NULL) {
        return false;
    }
    if (!gateway_packet_has_valid_payload_size(payload_len, hdr->fragment_id, hdr->fragment_count)) {
        return false;
    }
    if (g_reassembly.active == false || g_reassembly.frame_id != hdr->frame_id) {
        return false;
    }
    if (hdr->fragment_id >= COMPLETE_FRAME_FRAGMENTS) {
        return false;
    }
    if ((g_reassembly.received_mask & (1u << hdr->fragment_id)) != 0u) {
        return false;
    }

    const uint32_t data_len = (uint32_t)(payload_len - VIDEO_HEADER_FIELD_BYTES);
    const uint32_t offset = gateway_fragment_offset(hdr->fragment_id, hdr->fragment_count);
    const uint32_t remaining = g_reassembly.frame_size - offset;
    const uint32_t copy_len = (remaining < data_len) ? remaining : data_len;

    if (copy_len == 0u) {
        return false;
    }
    memcpy(g_reassembly.buffer + offset, payload + VIDEO_HEADER_FIELD_BYTES, copy_len);

    g_reassembly.received_mask |= (1u << hdr->fragment_id);
    g_reassembly.received_fragments++;
    return true;
}

static bool gateway_frame_is_complete(void) {
    if (!g_reassembly.active) {
        return false;
    }
    return g_reassembly.received_fragments == g_reassembly.fragment_count;
}

static bool gateway_submit_completed_frame(gateway_framebuffer_t *fb, uint8_t *frame_data) {
    if (fb == NULL || frame_data == NULL) {
        return false;
    }
    if (fb->back_ready) {
        return false;
    }
    memcpy(fb->back, frame_data, RAW_FRAME_SIZE);
    fb->back_ready = true;
    fb->latest_frame_id = g_reassembly.frame_id;
    return true;
}

static void gateway_swap_framebuffers(gateway_framebuffer_t *fb) {
    uint8_t *tmp = fb->front;
    fb->front = fb->back;
    fb->back = tmp;
    fb->back_ready = false;
    memset(fb->back, 0, RAW_FRAME_SIZE);
}

static gateway_video_status_t gateway_handle_udp_video_packet(gateway_framebuffer_t *fb,
                                                             const uint8_t *packet,
                                                             size_t packet_len) {
    if (fb == NULL || packet == NULL || packet_len == 0u) {
        return GATEWAY_VIDEO_BAD_LENGTH;
    }
    if (packet_len < VIDEO_HEADER_FIELD_BYTES) {
        return GATEWAY_VIDEO_BAD_LENGTH;
    }

    const video_frame_header_t *hdr = (const video_frame_header_t *)packet;
    if (hdr->type != VIDEO_FRAME) {
        return GATEWAY_VIDEO_BAD_TYPE;
    }
    if (hdr->version != 1u) {
        return GATEWAY_VIDEO_BAD_TYPE;
    }
    if (hdr->frame_size != RAW_FRAME_SIZE) {
        return GATEWAY_VIDEO_BAD_SIZE;
    }
    if (!gateway_packet_has_valid_payload_size(packet_len, hdr->fragment_id, hdr->fragment_count)) {
        return GATEWAY_VIDEO_BAD_LENGTH;
    }

    if (!g_reassembly.active) {
        gateway_video_status_t st = gateway_accept_new_frame(hdr->frame_id, hdr->fragment_count, hdr->frame_size);
        if (st != GATEWAY_VIDEO_OK) {
            return st;
        }
    } else if (hdr->frame_id != g_reassembly.frame_id) {
        /* The packet belongs to an older or newer frame. Defer or drop stale data. */
        if (hdr->frame_id < g_reassembly.frame_id) {
            return GATEWAY_VIDEO_OBSOLETE;
        }
        gateway_reset_reassembly();
        gateway_video_status_t st = gateway_accept_new_frame(hdr->frame_id, hdr->fragment_count, hdr->frame_size);
        if (st != GATEWAY_VIDEO_OK) {
            return st;
        }
    } else if (hdr->fragment_count != g_reassembly.fragment_count) {
        return GATEWAY_VIDEO_BAD_LENGTH;
    }

    if (!gateway_store_fragment(hdr, packet, packet_len)) {
        return GATEWAY_VIDEO_DUPLICATE;
    }

    if (gateway_frame_is_complete()) {
        if (!gateway_submit_completed_frame(fb, g_reassembly.buffer)) {
            gateway_reset_reassembly();
            return GATEWAY_VIDEO_INCOMPLETE;
        }
        gateway_swap_framebuffers(fb);
        gateway_reset_reassembly();
        return GATEWAY_VIDEO_READY;
    }

    return GATEWAY_VIDEO_INCOMPLETE;
}

/*
 * Frame reassembly rules for this UDP/39000 gateway:
 *  - Packet type is VIDEO_FRAME (20) and version must be 1.
 *  - Header is exactly 14 bytes:
 *      type(1) | version(1) | frame_id(2) | fragment_id(2) |
 *      fragment_count(2) | frame_size(4) | flags(2)
 *  - RGB565, 480x272 target raw frame size is 261,120 bytes.
 *  - The complete frame is split into 5 fragments because each UDP payload holds
 *    up to 59,986 bytes of pixel data. 5 x 59,986 ~= 299,930 bytes, while the
 *    raw frame is 261,120 bytes, so 5 fragments are sufficient.
 *  - Fragment id range is [0..4]. Receiver must accept only frame_id values that
 *    belong to the active assembly; stale/older ids are discarded.
 *  - A frame is complete only when all 5 fragment-bit masks are present and the
 *    total frame_size matches 261,120 bytes.
 *  - If a new frame id arrives before completion, drop the incomplete/obsolete
 *    frame and begin a fresh assembly.
 *  - Ignore duplicate fragments or packets whose fragment_count or size mismatch.
 *
 * Display pipeline guidance:
 *  - Use double buffering: front buffer = displayed frame, back buffer = newly
 *    assembled frame waiting to be swapped at the next vertical blank.
 *  - Copy the completed buffer into the back buffer and then swap only during a
 *    display-safe point. Never overwrite the front buffer while it is being read.
 *  - Keep the previous frame until the new one is fully reassembled; this avoids
 *    tearing and prevents a partially updated frame from being shown.
 *
 * Future H.264/compressed video path:
 *  - This gateway supports raw RGB565 now, but the protocol can evolve to a
 *    compressed video type that carries codec metadata (H.264 SPS/PPS/IDR frames)
 *    instead of raw image data. The receiver should then route compressed payloads
 *    through a decoder path, not the RGB565 reassembly path.
 */

int gateway_example_usage(void) {
    uint8_t front_buffer[RAW_FRAME_SIZE];
    uint8_t back_buffer[RAW_FRAME_SIZE];
    uint8_t packet[MAX_UDP_PAYLOAD_SIZE];
    video_frame_header_t *hdr = (video_frame_header_t *)packet;

    if (!gateway_init_framebuffer(&g_framebuffer, front_buffer, back_buffer)) {
        return 1;
    }

    memset(packet, 0, sizeof(packet));
    hdr->type = VIDEO_FRAME;
    hdr->version = 1u;
    hdr->frame_id = 42u;
    hdr->fragment_id = 0u;
    hdr->fragment_count = COMPLETE_FRAME_FRAGMENTS;
    hdr->frame_size = RAW_FRAME_SIZE;
    hdr->flags = 0u;

    /* Example packet processing. In a real gateway, this function would be called for
     * each packet received on UDP port 39000. */
    (void)gateway_handle_udp_video_packet(&g_framebuffer, packet, sizeof(video_frame_header_t));
    return 0;
}

