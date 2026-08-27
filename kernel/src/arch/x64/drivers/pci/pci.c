

#include "pci.h"
#include <lib/memory.h>
#include <stdint.h>
const uint16_t pci_config_address=0x0cf8;
const uint16_t pci_data_address=0x0cfc;
struct PCIConfig{
    uint16_t ManuFacturer;
    uint16_t DeviceId, Class,SubClass,HeaderType ;
    const char *vendor_name;
    const char *device_name;
} ;

static const struct PCIConfig device_table[] = {
    {0x8086, 0x29C0, 0,0,0,"Intel", "82G33/G31/P35/P31 DRAM Controller"},
    {0x8086, 0x10D3, 0,0,0,"Intel", "82574L Gigabit Network"},
    {0x8086, 0x2918, 0,0,0,"Intel", "ICH9 LPC Interface"},
    {0x8086, 0x2922, 0,0,0,"Intel", "ICH9 SATA Controller"},
    {0x8086, 0x2930, 0,0,0,"Intel", "ICH9 SMBus Controller"},
    {0x1234, 0x1111, 0,0,0,"QEMU", "Virtual VGA"},
    {0x1B36, 0x000D, 0,0,0,"QEMU", "xHCI USB Controller"},
    // 可以继续添加更多设备...
};
void print_pci_config(struct PCIConfig config){
    struct PCIConfig *found=NULL;
    for(int i=0;i<sizeof(device_table)/sizeof(struct PCIConfig);++i){
        if(config.ManuFacturer==device_table[i].ManuFacturer&&config.DeviceId==device_table[i].DeviceId){
            found=&device_table[i];
        }
    }
    if(found){
        serial_printk(found->vendor_name);
        write_serial(' ');
        serial_printk(found->device_name);
        write_serial('\n');
    }else{
        print_hex16(config.ManuFacturer);
        write_serial(' ');
        print_hex16(config.DeviceId);
        write_serial('\n');
    }
}


uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint64_t offset) { /* from OSDev.org*/
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;

    // Create configuration address as per Figure 1
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
    (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    // Write out the address
    outl(pci_config_address, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = (uint16_t)((inl(pci_data_address) >> ((offset & 2) * 8)) & 0xFFFF);
    return  inl(0xCFC);
}

uint16_t pciConfigReadDWord(uint8_t bus, uint8_t slot, uint8_t func, uint64_t offset) { /* from OSDev.org*/
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;

    // Create configuration address as per Figure 1
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
    (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    // Write out the address
    outl(pci_config_address, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    return  inl(0xCFC);
}

void scan_bus(uint64_t offset){
    uint16_t data=0;
    uint16_t func1=0;
    uint16_t pci=0;
    uint16_t busd;
    for(uint16_t bus=0;bus<=255;bus++){
        if ((busd  = pciConfigReadWord(bus,0,0,0 ))!=0xFFFF){for(uint8_t slot=0;slot<32;++slot){
            if ((data  = pciConfigReadWord(bus,slot,0,0 ))!=0xFFFF){
                for(uint8_t func=0;func<8;++func){
                    if((func1 = pciConfigReadWord(bus,slot,func,0 ))!=0xFFFF){

                        pci=pciConfigReadWord(bus,slot,func,offset);print_hex32(pci);

                    }

                }
            }

        }
        }
    }
}
 void scan_bus2(){
    struct PCIConfig pci_config;
    uint16_t data=0;
    uint16_t funcd=0;
    uint16_t pci=0;
    uint16_t busd=0;
    for(uint16_t bus=0;bus<=255;bus++){
        if ((busd  = pciConfigReadWord(bus,0,0,0 ))!=0xFFFF){for(uint8_t slot=0;slot<32;++slot){
            if ((data  = pciConfigReadWord(bus,slot,0,0 ))!=0xFFFF){
                for(uint8_t func=0;func<8;++func){
                    if((funcd = pciConfigReadWord(bus,slot,func,0 ))!=0xFFFF){
                        pci_config.ManuFacturer=pciConfigReadWord(bus,slot,func,0x00 );
                        pci_config.DeviceId=pciConfigReadWord(bus,slot,func,0x02 );
                        pci_config.Class=pciConfigReadWord(bus,slot,func,0x0A );
                        pci_config.SubClass=pciConfigReadWord(bus,slot,func,0x0B );
                        pci_config.HeaderType=pciConfigReadWord(bus,slot,func,0x0E );
                        print_itoa(bus);
                        write_serial(':');
                        print_itoa(slot);
                        write_serial('.');
                        print_itoa(func);
                        write_serial(' ');
                        print_pci_config(pci_config);
                    }

                }
            }

        }
        }
    }
}
// uint16_t scan_bus2(uint8_t offset){
//     uint16_t data=0;
//     uint16_t func1=0;
//     uint16_t pci=0;
//     for(uint32_t bus=0;bus<=255;bus++){
//         switch(pciConfigReadWord(bus,0,0,0)){
//             case 0x8086:
//                 serial_printk("intel,");
//                 break;
//             default:
//                 serial_printk("unknown,");
//                 break;}
//                 for(uint8_t slot=0;slot<32;++slot){
//                     switch(pciConfigReadWord(bus,slot,0,0)){
//                         case 0x0000:
//                             serial_printk("none,");
//                             break;
//                         default:
//                             serial_printk("unknown,");
//                             break;}
//                             for(uint8_t func=0;func<8;++func){
//                                 switch(pciConfigReadWord(bus,slot,func,0)){
//                                     case 0x0000:
//                                         serial_printk("none,");
//                                         break;
//                                     default:
//                                         serial_printk("unknown,");
//                                         break;}
//                             }
//                 }
//     return pci;
//
//     }
// }  似乎被我写坏了，算了，搞一个struct装这些吧
