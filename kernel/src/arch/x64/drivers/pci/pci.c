

#include "pci.h"
#include <lib/memory.h>
#include <stdint.h>
const uint16_t pci_config_address=0x0cf8;
const uint16_t pci_data_address=0x0cfc;
struct PCIConfig{
    uint16_t ManuFacturer;
    uint16_t DeviceId, Class,SubClass,HeaderType ;
} ;
void print_pci_config(struct PCIConfig config){
    switch(config.ManuFacturer){
        case 0x8086:
            serial_printk("intel ");

            break;
        default:
            serial_printk("unknown(");
            print_hex16(config.ManuFacturer);
            serial_printk(") ");
            break;}
        switch(config.DeviceId){
            default:
                serial_printk("unknow(");
                print_hex16(config.DeviceId);
                serial_printk(") ");
                break;
        }
            switch (config.HeaderType) {
                default:
                    serial_printk("\"idk what is the HeaerType mean,but i can give you binary:\"");
                    printf_hextob(config.HeaderType);
                    serial_printk("\n\r");
}

}
uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) { /* from OSDev.org*/
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
    return tmp;
}

void scan_bus(uint8_t offset){
    uint16_t data=0;
    uint16_t func1=0;
    uint16_t pci=0;
    uint16_t busd;
    for(uint16_t bus=0;bus<=255;bus++){
        if ((busd  = pciConfigReadWord(bus,0,0,0 ))!=0xFFFF){for(uint8_t slot=0;slot<32;++slot){
            if ((data  = pciConfigReadWord(bus,slot,0,0 ))!=0xFFFF){
                for(uint8_t func=0;func<8;++func){
                    if((func1 = pciConfigReadWord(bus,slot,func,0 ))!=0xFFFF){

                        pci=pciConfigReadWord(bus,slot,func,offset);print_hex16(pci);

                    }

                }
            }

        }
        }
    }
}
 void scan_bus_again(){
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
