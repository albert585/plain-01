#include <stdint.h>
#include "strings.h"
uint32_t kstrlen(const char * s){
    uint32_t i=0;
    while(s[i]!='\0'){i++;}
    return i;
}

int kstrcmp(const char* s1, const char* s2){
    while(*s1 ==*s2 && *s1!='\0'){
        s1++;
        s2++;
    }
    return *s1-*s2;
}

