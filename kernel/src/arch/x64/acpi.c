#include <limine.h>
#include <stdint.h>
#include "arch/x64/drivers/serial.h"
#include "mm/mem.h"                    /* get_hhdm_base */
#include "lib/stdio.h"                /* printf */
__attribute__((used, section(".limine_requests")))
static volatile struct limine_rsdp_request rsdp_request = LIMINE_RSDP_REQUEST;
struct acpi_sdt_header{
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision,creator_id,creator_revision;
}__attribute__((packed));

static uint8_t *acpi_to_virt(uint64_t phys){
    return (uint8_t *)(phys + get_hhdm_base());//ACPI里面是物理地址，将物理地址转换为虚拟地址

}

void acpi_init(void){
    if(!rsdp_request.response || !rsdp_request.response->address){ //判断acpi rsdp是否存在
        printf("ERR : NO ACPI RSDP\n");return; 
    }
    uint64_t addr =rsdp_request.response->address; //获取rsdp地址
    if(addr < 0x0008000000000000ULL){
        addr +=get_hhdm_base(); /*如果地址小于0x0008000000000000，则加上hhdm_base进行偏移，
                      * limine会将物理内存映射到高半区的虚拟地址空间中，也就是0xFFFF8xxxxxxx，
                      这样可以方便地访问物理内存。get_hhdm_base()函数返回的是高半区的基地址，
                      将物理地址加上这个基地址就可以得到对应的虚拟地址，从而在内核中访问ACPI表。
                      Limine 还是太方便了*/
    }
    uint8_t *rsdp = (uint8_t *)addr; //现在rsdp指向rsdp所在的地址了，读起来似乎有点绕啊
    if(rsdp[0] !='R' || rsdp[1] !='S' || rsdp[2] !='D' || rsdp[3]!=' '){
        printf("ERR:BAD ACPI RSDP IN %p",rsdp);return;
    }
    int is_xsdt = (rsdp[15] >= 2); /*判断是xsdt还是rsdt，
                       * ACPI 2.0及以上版本使用XSDT，而ACPI 1.0使用RSDT。
                       通过检查RSDP结构体中的Revision字段（位于偏移量15处）来确定使用哪种表。
                       如果Revision字段的值大于等于2，则表示使用XSDT，否则使用RSDT。*/
    uint64_t tbl_phys=is_xsdt?*(uint64_t *)(rsdp + 24):*(uint32_t *)(rsdp + 16); /* 获取XSDT或RSDT的物理地址。
                                               * 对于XSDT，物理地址位于RSDP结构体的偏移量24处（8字节），
                                               而对于RSDT，物理地址位于偏移量16处（4字节）。
                                               根据is_xsdt的值选择相应的偏移量，并将其解释为指向物理地址的指针，
                                               然后解引用该指针以获取实际的物理地址。*/
    if (!tbl_phys) { printf("ERR :no XSDT/RSDT\n"); return; }
    struct acpi_sdt_header *sdt = (struct acpi_sdt_header*)acpi_to_virt(tbl_phys); //将物理地址转为虚拟地址
    uint32_t n =(sdt->length - sizeof(*sdt)) / (is_xsdt ? 8 : 4); /*计算表项数量。
                       * SDT表头的长度字段（length）表示整个表的字节数，包括表头本身。
                       * 表头的大小为sizeof(*sdt)字节，因此减去表头大小后得到实际表项数据的字节数。
                       * 对于XSDT，每个表项占用8字节，而对于RSDT，每个表项占用4字节。
                       * 因此，将实际表项数据的字节数除以每个表项的大小即可得到表项数量。*/
    printf("Found ACPI %s with %d entries\n",is_xsdt?"XSDT":"RSDP",n);
    for (uint32_t i=0;i<n;i++){
        uint64_t p =is_xsdt?((uint64_t *)acpi_to_virt(tbl_phys+sizeof(*sdt)))[i]:((uint32_t*)acpi_to_virt(tbl_phys+sizeof(*sdt)))[i]; /*获取每个表项的物理地址。
                       * 对于XSDT，表项是64位指针，因此使用(uint64_t *)进行类型转换，
                       * 并通过索引[i]访问第i个表项。
                       * 对于RSDT，表项是32位指针，因此使用(uint32_t *)进行类型转换，
                       * 并通过索引[i]访问第i个表项。*/
        struct acpi_sdt_header *t = (struct acpi_sdt_header*) acpi_to_virt(p);//转化为虚拟地址
        for(int k=0;k<4;k++){write_serial(t->signature[k]);} //打印表头签名,因为signature是4字节的字符数组，所以循环打印每个字符
        printf(" NO.%u len=%u rev=%u oem=%.6s/%.8s phys=0x%llx\n",
            i, t->length, t->revision, t->oem_id, t->oem_table_id,
            (unsigned long long)p);}
    return;
}