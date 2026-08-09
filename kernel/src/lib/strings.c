#include <stdint.h>
#include "strings.h"
uint32_t strlen(const char * s){
    uint32_t i=0;
    while(s[i]!='\0'){i++;}
    return i;
}

