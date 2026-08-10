#include "arch/x64/drivers/serial.h"
#include <limine.h>
#include <stdbool.h>
#include <arch/x64/x64.h>
#include <stdint.h>
#include "lib/strings.h"
#include "arch/x64/drivers/framebuffer.h"
#include "arch/x64/gdt/gdt.h"
#include "arch/x64/idt/idt.h"
static char  * title="Plain,01\n";
extern void reloadSegments(void);

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

__attribute__((used,section(".limine_requests")))  volatile struct  limine_framebuffer_request framebuffer= LIMINE_FRAMEBUFFER_REQUEST;

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

void kmain()
{
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
    {
        hcf();
    }
    uint8_t *fb=(uint8_t*)framebuffer.response->framebuffers[0]->address;

    serial_init();
    serial_printk(title);
	struct RGBA c={111,111,111};
    for(uint32_t y=0; y<framebuffer.response->framebuffers[0]->height;++y){
        for(uint32_t x=0;x<framebuffer.response->framebuffers[0]->width;++x){
			draw_pixel(fb,x,y,c);

           // serial_printk("load\n", 27);
        }
    }
    // for(uint32_t y=100; y<200;++y){
    //     for(uint32_t x=10;x<200;++x){
    //         uint32_t offset = y*framebuffer.response->framebuffers[0]->pitch+x*4;
    //         fb[offset+0] = 255;
    //         fb[offset+1]=255;
    //         fb[offset+2]=255;
    //
    //         // serial_printk("load\n", 27);
    //     }
    // }
    // int ret=drawfont(fb,100,200,'b');
    // if (ret==1){serial_printk("OK\n",strlen("OK"));}
    //draw_string(fb,100,200 ,"The quick brown fox jumps over the lazy dog." );
    // 小写全字母
    draw_string(fb,0 , 0, "Hello Plain,01");
    draw_string(fb, 100, 200, "abcdefghijklmnopqrstuvwxyz");
    // 大写全字母
    draw_string(fb, 100, 220, "ABCDEFGHIJ\nKLMNOPQRSTUVWXYZ");
    load_gdt(); //加载自己的GDT
    reloadSegments();
    // 数字 + 标点
    //draw_string(fb, 100, 240, "0123456789 !@#$%^&*()");
    // 先设 IDT
    set_idt_entry(0x20, isr_wrapper, 0x08, 0x8E);  // 使用了 0x08
    load_idt();
    asm("int $0x20");   // 测试：触发软件中断
    int i=0;
    int n=0;
    char buf[256];
    while(1){
        char c=read_serial();
        if(c){
            if(c=='\r'){
                buf[i]='\0';
                if(!(kstrcmp(buf,"uname"))){
                    serial_printk("\n\r");
                    serial_printk("Plain-01 prototype d ");
                    draw_string(fb,0,16*(++n),"A toy kernel based on OSDev and limine");
                    serial_printk("\n\r");
                } else if(!(kstrcmp(buf,"cat"))){
                    serial_printk("\n\r");
                    serial_printk("哈基米南北绿豆");
                    serial_printk("\n\r");
                }
                else{
                    serial_printk("\n\r");
                    serial_printk(buf);
                    serial_printk("\n\r");
                    draw_string(fb,0,16*(++n),buf);
                }
                for(int m=0;m<=i;m++){buf[m]='\0';}//很粗暴的初始化 没有memcpy只能这样子了
                i=0;
            }else if(c==0x7F && i>0){
                i--;
                buf[i]='\0';
                write_serial('\b');
                write_serial(' ');
                write_serial('\b');
            }
            else{
                buf[i]=c;
                i++;
                write_serial(c);
            }

        }
    }
    hcf();
}
