#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include <stdint.h>
#include "fonts/font.h"
#include <limine.h>

extern const struct bitmap_font font;
extern volatile struct limine_framebuffer_request framebuffer;
struct RGBA{
    uint8_t r,g,b;
} ;


extern int drawfont(uint8_t* fb,uint32_t x,uint32_t y,    uint64_t ch);
extern void draw_string(uint8_t* fb,uint32_t x,uint32_t y,const char *s);
extern void draw_pixel(uint8_t * fb, uint32_t x,uint32_t y, struct RGBA c);
#endif
