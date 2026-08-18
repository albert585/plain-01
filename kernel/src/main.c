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
#include "mm/hhdm.h"
#include "mm/frame.h"
#include "arch/x64/mm/page_table.h"
__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used,section(".limine_requests")))  volatile struct  limine_framebuffer_request framebuffer= LIMINE_FRAMEBUFFER_REQUEST;
/* memmap / hhdm 请求已移到 kernel/src/mm/ 下 */
__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

uint64_t mem;
static char  * title="Plain,01\n";
extern void division_error_wrapper(void);
extern void page_fault_wrapper(void);
static inline void cli(void) {
    asm volatile("cli");
}

static inline void sti(void) {
    asm volatile("sti");
}
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
unsigned read_pit_count(void) {
    unsigned count = 0;
    
    // 禁用中断
    cli();
    
    // al = 位6和7中的通道，其余位清零
    outb(0x43, 0b0000000); 
    
    count = inb(0x40);      // 低字节
    count |= inb(0x40) << 8; // 高字节
    sti();
    return count;
}
uint64_t mem;
void kmain()
{
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
    {
        hcf();
    }

    uint8_t *fb=(uint8_t*)framebuffer.response->framebuffers[0]->address;

    serial_init();

    hhdm_init();
    frame_init();
    sse_start();
    serial_printk(title);
    
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
    set_idt_entry(0x00,division_error_wrapper,0x08,0x8E,0);
    set_idt_entry(0x08,double_fault_wrapper,0x08,0x8E,0);
    set_idt_entry(0x20, isr_wrapper, 0x08, 0x8E,0);  // 使用了 0x08
    set_idt_entry(0x0E, page_fault_wrapper, 0x08, 0x8E,1);
    load_idt();
    init_pic();
    //asm("int $0x20");   // 测试：触发软件中断
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
                        print_itoa(read_pit_count());
                        serial_printk("\n\r");
                }else if(!(kstrcmp(buf,"df"))){
                    *((volatile uint64_t*)0xDEADBEEF)=0x114514;
                }else if(!(kstrcmp(buf,"cpuid"))){
                    serial_printk("\n\r");
                    get_model();
                }else if(!(kstrcmp(buf,"mem"))){
                    serial_printk("\n\r");
                    serial_printk("HHDM offset: 0x");
                    print_hex64(hhdm_offset);
                    serial_printk("\n\rframes total/free: ");
                    print_itoa(pmm.total_frames);
                    serial_printk(" / ");
                    print_itoa(pmm.free_frames);
                    serial_printk("\n\r");
                    /* 自测：分配 4 帧，写一个值，释放 */
                    uint64_t p = frame_alloc(4);
                    serial_printk("alloc 4 frames @0x");
                    print_hex64(p);
                    serial_printk(", free now: ");
                    print_itoa(pmm.free_frames);
                    serial_printk("\n\r");
                    volatile uint64_t *v = (volatile uint64_t *)phys_to_virt(p);
                    *v = 0x1234;
                    serial_printk("write via HHDM: 0x");
                    print_hex64(*v);
                    serial_printk("\n\r");
                    frame_free(p, 4);
                    serial_printk("freed, free now: ");
                    print_itoa(pmm.free_frames);
                    serial_printk("\n\r");
                }else if(!(kstrcmp(buf,"pt"))){
                    serial_printk("\n\rcurrent PML4 @0x");
                    print_hex64((uint64_t)get_current_pml4());
                    serial_printk("\n\r");
                    /* 自测：建一棵独立页表，映射一个测试虚拟地址 */
                    uint64_t *test_pml4 = (uint64_t *)phys_to_virt(frame_alloc(1));
                    uint64_t  test_phys = frame_alloc(1);
                    const uint64_t TEST_VA = 0xFFFF900000000000ULL;
                    map_page(test_pml4, TEST_VA, test_phys, PF_WRITE);
                    volatile uint64_t *tv = (volatile uint64_t *)TEST_VA;
                    serial_printk("test pml4 @0x");
                    print_hex64((uint64_t)test_pml4);
                    serial_printk(", test frame @0x");
                    print_hex64(test_phys);
                    serial_printk("\n\rmap_page done. (NOT touching CR3)\n\r");
                    /* 现在还没切 CR3，TEST_VA 不可访问；验证页表项内容 */
                    uint64_t pte = 0;
                    {
                        uint64_t *t = test_pml4;
                        uint64_t idx;
                        for (int level = 0; level < 3; ++level) {
                            idx = (TEST_VA >> (39 - 9*level)) & 0x1FF;
                            t = (uint64_t *)phys_to_virt(t[idx] & PTE_ADDR_MASK);
                        }
                        idx = (TEST_VA >> 12) & 0x1FF;
                        pte = t[idx];
                    }
                    serial_printk("PTE = 0x");
                    print_hex64(pte);
                    serial_printk(" (expect phys | P|W|NX)\n\r");
                    unmap_page(test_pml4, TEST_VA);
                    serial_printk("unmapped. free frames: ");
                    print_itoa(pmm.free_frames);
                    serial_printk("\n\r");
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
