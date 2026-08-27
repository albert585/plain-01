#ifndef PLAIN_MM_MEM_H
#define PLAIN_MM_MEM_H

#include <stdint.h>

/* 内核高半区保留地址，专门给 PCI 设备 MMIO 作手动映射。
 * 位于 Limine 内核镜像 (0xFFFFFFFF80000000..) 与 HHDM (0xFFFF800000000000..) 之间，
 * 是一个 Limine 未占用的高半区地址。
 */
#define MMIO_VBASE 0xFFFFC00000000000ULL

/* 页表项/页标志位 */
#define P_PRESENT (1ULL << 0)
#define P_WRITE   (1ULL << 1)
#define P_USER    (1ULL << 2)
#define P_NX      (1ULL << 63)

/* 初始化物理页分配池，hhdm_offset 为 Limine HHDM 请求返回的 offset */
void mm_init(uint64_t hhdm_offset);

/* 分配一个已清零的 4K 物理页（bump），返回物理地址；
 * 经 HHDM 虚拟地址（phys + get_hhdm_base()）访问该页。 */
uint64_t pmm_alloc_zero(void);

/* Limine HHDM offset；把物理地址转成可访问的虚拟地址用：virt = phys + base */
uint64_t get_hhdm_base(void);

/* 当前 CR3（PML4 物理地址），用于页表遍历起点 */
uint64_t get_cr3(void);
#endif
