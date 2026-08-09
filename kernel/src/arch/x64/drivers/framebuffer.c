#include "framebuffer.h"
int drawfont(uint8_t* fb,uint32_t x,uint32_t y,    uint64_t ch){
    int idx,found;
    idx=0;
    found=0;
    struct RGBA c={255,255,255};
    for(int i=0;i<font.Chars;i++){
        if(font.Index[i]==ch){idx=i;found=1;break;}
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
void draw_string(uint8_t* fb,uint32_t x,uint32_t y,const char *s){
    int count=0;
    for(int i=0;s[i]!='\0';++i){
        count++;
        if(s[i]=='\n'){y+=16;count=0;continue;}
        drawfont(fb,x+8*count,y,s[i]);
    }
}
void draw_pixel(uint8_t * fb, uint32_t x,uint32_t y, struct RGBA c){
    uint32_t offset = y*framebuffer.response->framebuffers[0]->pitch+x*4;
    fb[offset+0]= c.r;
    fb[offset+1]=c.g;
    fb[offset+2]=c.b;
}
