#include <stdint.h>
#include "arch/x64/drivers/serial.h"
#include <lib/memory.h>

uint16_t scan_bus(uint8_t offset);
//uint16_t scan_bus2(uint8_t offset);
static void print_hex16(uint16_t val){
    char buf[6];
    for(int i=3;i>=0;--i){
        uint8_t temp = val& 0xF;
        buf[i]=temp<10 ? '0'+temp:'A'+(temp-10);
        val>>=4;
    }
    buf[4]=' ';
    buf[5]='\0';
    serial_printk(buf);
}
static void print_itoa(uint64_t val){
    char buf[22];
    kmemset(buf, 0, 22);
    int i=20;
    buf[i]='\0';
    uint64_t temp=0;
    if (val == 0){serial_printk("0");}
    else{
        while(val>0){
            temp = val %10;
            buf[--i]='0'+temp;
            val/=10;
        }
    }
    while(val>0){
        temp = val %10;
        buf[--i]='0'+temp;
        val/=10;
    }
    serial_printk(&buf[i]);
}
