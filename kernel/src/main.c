#include "arch/x64/drivers/serial.h"
#include <limine.h>
#include <stdbool.h>
#include <arch/x64/x64.h>
#include <stdint.h>
#include "fonts/font.h"

struct RGBA{
	uint8_t r,g,b;
} ;


void draw_pixel(uint8_t * fb, uint32_t y,uint32_t x,struct RGBA c);
extern const struct bitmap_font font;




// struct bitmap_font {
// 	unsigned char Width;		///< max. character width
// 	unsigned char Height;		///< character height
// 	unsigned short Chars;		///< number of characters in font
// 	const unsigned char *Widths;	///< width of each character
// 	const unsigned short *Index;	///< encoding to character index
// 	const unsigned char *Bitmap;	///< bitmap of all characters
// };
__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used,section(".limine_requests"))) static volatile struct  limine_framebuffer_request framebuffer= LIMINE_FRAMEBUFFER_REQUEST;

__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

// Halt and catch fire function.
static void hcf(void)
{
    for (;;)
    {
        asm("hlt");

    }
}
// kernelmain
int drawfont(uint8_t* fb,uint32_t x,uint32_t y,    uint64_t codepoint){
    int idx,found;
    idx=0;
    found=0;
	struct RGBA c;
	c.r=255;c.b=255;c.g=255;
    for(int i=0;i<font.Chars;i++){
        if(font.Index[i]==codepoint){idx=i;found=1;break;}
    }
    if(found==1){
        const uint8_t* data=&font.Bitmap[idx*font.Height];
        for (uint32_t dy=0;dy<font.Height;++dy){
            for (uint32_t dx=0;dx<font.Width;++dx){
                if(data[dy] & (0x80u>>dx)){draw_pixel(fb,x+dx,y+dy,c);
                }

            }
        }
    }
	return found;
}
void kmain()
{
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
    {
        hcf();
    }
    uint8_t *fb=(uint8_t*)framebuffer.response->framebuffers[0]->address;

    serial_init();
    serial_printk("Plain,01\n", 27);
	struct RGBA c;
	c.r=111;
	c.b=111;
	c.g=111;
    for(uint32_t y=0; y<framebuffer.response->framebuffers[0]->height;++y){
        for(uint32_t x=0;x<framebuffer.response->framebuffers[0]->width;++x){
			draw_pixel(fb,x,y,c);

           // serial_printk("load\n", 27);
        }
    }
    for(uint32_t y=100; y<200;++y){
        for(uint32_t x=10;x<200;++x){
            uint32_t offset = y*framebuffer.response->framebuffers[0]->pitch+x*4;
            fb[offset+0] = 255;
            fb[offset+1]=255;
            fb[offset+2]=255;

            // serial_printk("load\n", 27);
        }
    }
    int ret=drawfont(fb,100,200,'b');
    if (ret==1){serial_printk("OK",2);}
    hcf();
}
void draw_pixel(uint8_t * fb, uint32_t x,uint32_t y, struct RGBA c){
	uint32_t offset = y*framebuffer.response->framebuffers[0]->pitch+x*4;
	fb[offset+0]= c.r;
	fb[offset+1]=c.g;
	fb[offset+2]=c.b;
}
