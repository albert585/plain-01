
#include "arch/x64/drivers/framebuffer.h"
#include <limine.h>
#include <stdint.h> 
#include "lib/strings.h"
#include "lib/memory.h"


uint8_t *fb;

uint32_t width;
uint32_t height;
struct position{
    int x;
    int y;
} Pos ;//然后我在想怎么搞console,我的设想是在没有到底的时候正常遇到\n换行换行，如果到底了就利用memcpy（因为我就实现了一个memcpy）拷贝到上一级，最后一行memset填空以后写入新值

void console_newline();
void console_write(const char *str){
    fb=(uint8_t*)framebuffer.response->framebuffers[0]->address;
    Pos.x=0;
    Pos.y=0;
    int col=0;
    for(int j=0;str[j]!='\0';++j){
        if(str[j]=='\n'){
            Pos.x=0;
            col=0;
            console_newline();

        }else{
            draw_font(fb, Pos.x+col*8 ,Pos.y,str[j]);
            col++;
        }
    }
}
void console_newline(){
    //先拿到高宽
    width=framebuffer.response->framebuffers[0]->pitch;
    height=framebuffer.response->framebuffers[0]->height;
    if(Pos.y<=height){
         Pos.y+=16;

    }else{
        for(int i=0;i<height;i++){
            kmemcpy(fb+(Pos.y-16)*width,fb+Pos.y*width,sizeof(fb+(Pos.y-16)*width));
            kmemset(fb+Pos.y*width,0,sizeof(fb+Pos.y*width));
        }
    }
}