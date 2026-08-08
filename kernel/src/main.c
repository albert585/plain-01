#include <limine.h>
#include <stdbool.h>
#include <arch/x64/x64.h>
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
void kmain()
{
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
    {
        hcf();
    }
    uint8_t *fb=(uint8_t*)framebuffer.response->framebuffers[0]->address;

    serial_init();
    serial_printk("Plain,01\n", 27);
    hcf();
    for(uint32_t y=0; y<framebuffer.response->framebuffers[0]->height;++y){
        for(uint32_t x=0;x<framebuffer.response->framebuffers[0]->width;++x){
            uint32_t offset = (y * framebuffer.response->framebuffers[0]->pitch + x) * 4;
            fb[offset+0] = 111;
            fb[offset+1]=111;
            fb[offset+2]=111;
           // serial_printk("load\n", 27);
        }
    }
}
