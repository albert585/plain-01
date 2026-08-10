#include <stdint.h>
// struct gdt  {
//     uint16_t limit;          // limit 15:0
//     uint16_t base_low;       // base 15:0
//     uint8_t  base_mid;       // base 23:16
//     uint8_t  access;         // S=0, type=0x9 (64位TSS)
//     uint8_t  flags_limit;    // G + limit 19:16
//     uint8_t  base_high;      // base 31:24
//     uint32_t base_upper;     // base 63:32 ← 你写的 base_fin
//     uint32_t reserved;       // ← 补这个，必须 0
// } __attribute__((packed));
// /*
// struct tss {
//     uint32_t reserved0;          // 0x00
//     uint64_t rsp0;               // 0x04  ← ring 0 栈指针（最重要的字段）
//     uint64_t rsp1;               // 0x0C
//     uint64_t rsp2;               // 0x14
//     uint64_t reserved1;          // 0x1C
//     uint64_t ist[7];             // 0x24  ← 中断栈表
//     uint64_t reserved2;          // 0x5C
//     uint16_t reserved3;          // 0x64
//     uint16_t iomap_base;         // 0x66
// } __attribute__((packed));
//
// static struct tss tss;
// void set_segment(struct tss tss){
//     tss.rsp0=
//} 目前暂时用静态的，动态的后面再说吧
uint64_t gdt[16]=  {
    0x0000000000000000,
    0x00AF9A000000FFFF,
    0x00CF92000000FFFF,
    0x00AFFA000000FFFF,
    0x00CFF2000000FFFF,
};
struct gdtr_t{
    uint16_t limit;
    uint64_t base;
}__attribute__((packed)) ;
struct gdtr_t gdtr={sizeof(gdt)-1,(uint64_t)gdt};

void load_gdt(void)
{
    asm volatile("lgdt %0" :: "m"(gdtr) : "memory");
}

