

#include "pci.h"

#include <stdint.h>
const uint16_t pci_config_address=0x0cf8;
const uint16_t pci_data_address=0x0cfc;
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
void print_hex32(uint32_t val) {
    char buf[9];
    for (int i = 7; i >= 0; i--) {
        uint8_t nib = val & 0xF;
        buf[i] = nib < 10 ? '0' + nib : 'A' + (nib - 10);
        val >>= 4;
    }
    buf[8] = '\0';
    serial_printk(buf);
}
uint16_t scan_bus(){
    uint16_t data=0;
    uint16_t pci=0;
    for(uint8_t bus=0;bus<=255;bus++){
        for(uint8_t slot=0;slot<8;++slot){
            if ((data  = pciConfigReadWord(bus,slot,0,0 ))!=0xFFFF){
                for(uint8_t func=0;func<8;++func){
                    pci=pciConfigReadWord(bus,slot,func,0 );
                    print_hex32(pci);
                }
            }

        }
    }
    return pci;
}
