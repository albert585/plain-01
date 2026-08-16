#include "arch/x64/drivers/serial.h"
#include <limine.h>
#include <stdbool.h>
#include <arch/x64/x64.h>
#include <stdint.h>
#include "lib/strings.h"
#include "arch/x64/drivers/framebuffer.h"
#include "arch/x64/drivers/pci/pci.h"
#include "arch/x64/gdt/gdt.h"
#include "arch/x64/idt/idt.h"
static char  * title="Plain,01\n";
extern void division_error_wrapper(void);
extern void sse_start(void);
extern void reloadSegments(void);
extern void init_pic(void);
extern void check_cpuid(void);
void err_not_support(uint8_t a){
    switch(a){
        case 1:
            serial_printk("your CPU don`t support SSE");
            break;
        default:
            break;
    }
}
void print_hex64(uint64_t val){
    char buf[18];
    for(int i=15;i>=0;--i){
        uint8_t temp = val& 0xF;
        buf[i]=temp<10 ? '0'+temp:'A'+(temp-10);
        val>>=4;
    }
    buf[16]='\0';
    serial_printk(buf);
}

static void get_model(void)
{
    uint64_t rbx, rcx,rdx,unused;
    char str[14]={0};
    __cpuid(0, unused, rbx, rcx, rdx);
    for(int i=0;i<=3;++i){
        str[i]=rbx>>(i*8)&0xFF;
    }
    for(int i=4;i<=7;++i){
        str[i]=rdx>>((i-4)*8)&0xFF;
    }
    for(int i=8;i<=11;++i){
        str[i]=rcx>>((i-8)*8)&0xFF;
    }
    str[12]='\n';
    str[13]='\0';
    serial_printk(str);
}

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
void trigger_divide_error(void) {
    volatile int a = 10;
    volatile int b = 0;

    // 方法1：直接用汇编
    __asm__ volatile(
        "movl %[a], %%eax\n\t"
        "movl %[b], %%ebx\n\t"
        "divl %%ebx\n\t"      // 这里的 ebx=0，CPU 触发 #DE
        :
        : [a] "m" (a), [b] "m" (b)
        : "eax", "ebx", "cc"
    );
}
void kmain()
{
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
    {
        hcf();
    }

    uint8_t *fb=(uint8_t*)framebuffer.response->framebuffers[0]->address;

    serial_init();
    sse_start();
    serial_printk(title);
    init_pic();
    uintptr_t rsp;
    asm("mov %%rsp, %0" : "=r"(rsp));
    print_itoa(rsp % 16);   // 打印 0 是对齐，非 0 是不对齐
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
    set_idt_entry(0x00,division_error_wrapper,0x08,0x8E);
    set_idt_entry(0x08,double_fault_wrapper,0x08,0x8E);
    set_idt_entry(0x20, isr_wrapper, 0x08, 0x8E);  // 使用了 0x08
    load_idt();
    asm("int $0x20");   // 测试：触发软件中断
    int i=0;
    int n=0;
    char buf[256]={0};
    write_serial('\n');
    //for(int m=0;m<256;m++){buf[m]='\0';}//很粗暴的初始化x2
    while(1){
        if(i==0){write_serial('>');}
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
                } else if(!(kstrcmp(buf,"de"))){
                        trigger_divide_error();

                }else if(!(kstrcmp(buf,"df"))){
                    *((volatile uint64_t*)0xDEADBEEF)=0x114514;
                }else if(!(kstrcmp(buf,"cpuid"))){
                    serial_printk("\n\r");
                    get_model();
                }else if(!(kstrcmp(buf,"pci"))){
                    // serial_printk("\n\r");
                    // serial_printk("offset 0x00:");
                    // scan_bus(0x00);
                    // serial_printk("\n\r");
                    // serial_printk("offset 0x08:");
                    // scan_bus(0x08);
                    // serial_printk("\n\r");
                    // serial_printk("offset 0x09:");
                    // scan_bus(0x09);
                    // serial_printk("\n\r");
                    // serial_printk("offset 0x0A:");
                    // scan_bus(0x0A);
                    // serial_printk("\n\r");}
                    serial_printk("\n\r");
                    scan_bus_again();}
                else{
                    serial_printk("\n\r");
                    serial_printk("unknown command");
                    serial_printk("\n\r");
                }
                for(int m=0;m<=i;m++){buf[m]='\0';}
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
