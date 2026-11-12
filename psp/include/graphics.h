#ifndef PSP_ICE_GRAPHICS_H
#define PSP_ICE_GRAPHICS_H

int ice_gfx_init(void);
void ice_gfx_begin(void);
void ice_gfx_rect(int x, int y, int w, int h, unsigned int color);
void ice_gfx_text(int x, int y, const char *text, int scale,
                  unsigned int color);
void ice_gfx_end(void);
void ice_gfx_shutdown(void);

#endif
