#include "memory.h"
void kmemcpy(void *dst,void const * src,size_t n){
    char * d=dst;
    char  const * s=src;
    for(int i=0;i<n;++i){
        d[i]=s[i];
    }
}
void kmemset(void *dst,const char c,size_t n){
    char * d=dst;
    for(int i=0;i<n;++i){
        d[i]=c;
    }
}
