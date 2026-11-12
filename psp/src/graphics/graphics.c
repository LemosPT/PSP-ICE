#include <pspgu.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <string.h>

#include "graphics.h"

#define SCREEN_W 480
#define SCREEN_H 272
#define BUFFER_W 512
#define LIST_SIZE (256 * 1024)

typedef struct {
    float x, y, z;
    unsigned int color;
} gfx_vertex_t;

static unsigned int __attribute__((aligned(16))) display_list[LIST_SIZE / 4];
static gfx_vertex_t __attribute__((aligned(16))) text_vertices[4096];
static unsigned int text_vertex_count;

static const char font_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-:./%";
static const unsigned char font[41][5] = {
    {0x7E,0x11,0x11,0x11,0x7E}, {0x7F,0x49,0x49,0x49,0x36},
    {0x3E,0x41,0x41,0x41,0x22}, {0x7F,0x41,0x41,0x22,0x1C},
    {0x7F,0x49,0x49,0x49,0x41}, {0x7F,0x09,0x09,0x09,0x01},
    {0x3E,0x41,0x49,0x49,0x7A}, {0x7F,0x08,0x08,0x08,0x7F},
    {0x00,0x41,0x7F,0x41,0x00}, {0x20,0x40,0x41,0x3F,0x01},
    {0x7F,0x08,0x14,0x22,0x41}, {0x7F,0x40,0x40,0x40,0x40},
    {0x7F,0x02,0x0C,0x02,0x7F}, {0x7F,0x04,0x08,0x10,0x7F},
    {0x3E,0x41,0x41,0x41,0x3E}, {0x7F,0x09,0x09,0x09,0x06},
    {0x3E,0x41,0x51,0x21,0x5E}, {0x7F,0x09,0x19,0x29,0x46},
    {0x46,0x49,0x49,0x49,0x31}, {0x01,0x01,0x7F,0x01,0x01},
    {0x3F,0x40,0x40,0x40,0x3F}, {0x1F,0x20,0x40,0x20,0x1F},
    {0x3F,0x40,0x38,0x40,0x3F}, {0x63,0x14,0x08,0x14,0x63},
    {0x07,0x08,0x70,0x08,0x07}, {0x61,0x51,0x49,0x45,0x43},
    {0x3E,0x45,0x49,0x51,0x3E}, {0x00,0x42,0x7F,0x40,0x00},
    {0x62,0x51,0x49,0x49,0x46}, {0x22,0x41,0x49,0x49,0x36},
    {0x18,0x14,0x12,0x7F,0x10}, {0x2F,0x49,0x49,0x49,0x31},
    {0x3E,0x49,0x49,0x49,0x32}, {0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36}, {0x26,0x49,0x49,0x49,0x3E},
    {0x08,0x08,0x08,0x08,0x08}, {0x00,0x36,0x36,0x00,0x00},
    {0x00,0x60,0x60,0x00,0x00}, {0x40,0x20,0x10,0x08,0x04},
    {0x63,0x13,0x08,0x64,0x63}
};

static void draw_rect_now(int x, int y, int w, int h, unsigned int color) {
    gfx_vertex_t vertices[2] __attribute__((aligned(16)));
    vertices[0].x = (float)x;     vertices[0].y = (float)y;     vertices[0].z = 0.0f;
    vertices[0].color = color;
    vertices[1].x = (float)(x+w); vertices[1].y = (float)(y+h); vertices[1].z = 0.0f;
    vertices[1].color = color;
    sceKernelDcacheWritebackRange(vertices, sizeof(vertices));
    sceGuDrawArray(GU_SPRITES,
        GU_VERTEX_32BITF | GU_COLOR_8888 | GU_TRANSFORM_2D,
        2, NULL, vertices);
}

int ice_gfx_init(void) {
    sceGuInit();
    sceGuStart(GU_DIRECT, display_list);
    sceGuDrawBuffer(GU_PSM_8888, (void *)0, BUFFER_W);
    sceGuDispBuffer(SCREEN_W, SCREEN_H, (void *)0x88000, BUFFER_W);
    sceGuDepthBuffer((void *)0x110000, BUFFER_W);
    sceGuOffset(2048 - (SCREEN_W / 2), 2048 - (SCREEN_H / 2));
    sceGuViewport(2048, 2048, SCREEN_W, SCREEN_H);
    sceGuDepthRange(65535, 0);
    sceGuScissor(0, 0, SCREEN_W, SCREEN_H);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuDisable(GU_DEPTH_TEST);
    sceGuShadeModel(GU_SMOOTH);
    sceGuClearColor(0xFF080D14);
    sceGuClear(GU_COLOR_BUFFER_BIT);
    sceGuFinish();
    sceGuSync(0, 0);
    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);
    return 0;
}

void ice_gfx_begin(void) {
    sceGuStart(GU_DIRECT, display_list);
    sceGuClearColor(0xFF080D14);
    sceGuClear(GU_COLOR_BUFFER_BIT);
    text_vertex_count = 0;
}

void ice_gfx_rect(int x, int y, int w, int h, unsigned int color) {
    draw_rect_now(x, y, w, h, color);
}

void ice_gfx_text(int x, int y, const char *text, int scale,
                  unsigned int color) {
    int cursor = x;
    if (!text || scale < 1) return;

    while (*text && text_vertex_count + 70 < 4096) {
        char c = *text++;
        const unsigned char *glyph = NULL;
        int i, row;

        if (c == ' ') {
            cursor += 4 * scale;
            continue;
        }

        for (i = 0; i < (int)sizeof(font_chars) - 1; i++) {
            if (font_chars[i] == c) {
                glyph = font[i];
                break;
            }
        }

        if (!glyph) {
            cursor += 6 * scale;
            continue;
        }

        for (i = 0; i < 5; i++) {
            for (row = 0; row < 7; row++) {
                if (glyph[i] & (1u << row)) {
                    gfx_vertex_t *a = &text_vertices[text_vertex_count++];
                    gfx_vertex_t *b = &text_vertices[text_vertex_count++];
                    a->x = (float)(cursor + i * scale);
                    a->y = (float)(y + row * scale);
                    a->z = 0.0f;
                    a->color = color;
                    b->x = a->x + (float)scale;
                    b->y = a->y + (float)scale;
                    b->z = 0.0f;
                    b->color = color;
                }
            }
        }
        cursor += 6 * scale;
    }
}

void ice_gfx_end(void) {
    if (text_vertex_count > 0) {
        sceKernelDcacheWritebackRange(text_vertices,
                                      text_vertex_count * sizeof(gfx_vertex_t));
        sceGuDrawArray(GU_SPRITES,
            GU_VERTEX_32BITF | GU_COLOR_8888 | GU_TRANSFORM_2D,
            text_vertex_count, NULL, text_vertices);
    }
    sceGuFinish();
    sceGuSync(0, 0);
    sceDisplayWaitVblankStart();
    sceGuSwapBuffers();
}

void ice_gfx_shutdown(void) {
    sceGuTerm();
}
