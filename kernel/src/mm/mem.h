#ifndef PLAIN_MM_MEM_H
#define PLAIN_MM_MEM_H

#include <stdint.h>

/* 内核高半区保留地址，专门给 PCI 设备 MMIO 作手动映射。
 * 位于 Limine 内核镜像 (0xFFFFFFFF80000000..) 与 HHDM (0xFFFF800000000000..) 之间，
 * 是一个 Limine 未占用的高半区地址。
 */
#define MMIO_VBASE 0xFFFFC00000000000ULL

/* 页表项 / 页标志位（对应 x86-64 页表项的低位与第 63 位）
 * 这里只定义本内核当前用到的几个，未列出的位一律保持 0 */
#define P_PRESENT (1ULL << 0)  /* bit0  P：1 = 该项有效，映射已建立 */
#define P_WRITE   (1ULL << 1)  /* bit1  R/W：1 = 可写，0 = 只读 */
#define P_USER    (1ULL << 2)  /* bit2  U/S：1 = 用户态可访问，0 = 仅内核 */
#define P_NX      (1ULL << 63) /* bit63 NX：1 = 该页不可取指令（数据页用） */

/* 初始化物理页分配池：记录 HHDM offset，并在 Limine 内存地图里挑一段
 * 可用的物理 RAM 作为 bump 分配的起点。
 * hhdm_offset 来自 Limine HHDM 请求；调用前必须先拿到它，否则物理地址无法访问。 */
void mm_init(uint64_t hhdm_offset);

/* 分配一个已清零的 4K 物理页（bump 分配，只进不退）。
 * 返回【物理地址】；要读写这页必须经 HHDM 换算：virt = phys + get_hhdm_base()。 */
uint64_t pmm_alloc_zero(void);

/* Limine HHDM offset；把物理地址转成可访问的虚拟地址用：virt = phys + base */
uint64_t get_hhdm_base(void);

/* 读当前 CR3 寄存器，即 PML4 表的物理地址；页表遍历从这里开始 */
uint64_t get_cr3(void);
#endif
