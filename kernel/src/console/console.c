
#include "arch/x64/drivers/framebuffer.h"
#include <limine.h>
#include <stdint.h> 
#include "lib/strings.h"
#include "lib/memory.h"


uint8_t *fb;

uint32_t width;
uint32_t height;
int col=0;
int line=0;
void console_newline();
void console_write(const char *str){
    fb=(uint8_t*)framebuffer.response->framebuffers[0]->address;

    for(int j=0;str[j]!='\0';++j){
        if(str[j]=='\n'){

            console_newline();

        }else{
            draw_font(fb, col*8 ,line*16,str[j]);
            col++;
        }
    }
}
void console_newline(){
    //先拿到高宽
    width=framebuffer.response->framebuffers[0]->width;
    height=framebuffer.response->framebuffers[0]->height;
    if(line<=height){
         line++;
         col=0;

    }else{
        for(int i=0;i<height;i++){
            // kmemcpy(fb+(Pos.y-16)*width,fb+Pos.y*width,sizeof(fb+(Pos.y-16)*width));
            // kmemset(fb+Pos.y*width,0,sizeof(fb+Pos.y*width));
        }
    }
}