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

void *kmalloc(size_t n){
    n =(n+15) & ~15ULL; //16字节对齐
    if(heap_cur +n >heap_end){return 0;}
    uint64_t first_page=heap_cur & ~0xFFFULL;
    uint64_t last_page = (heap_cur + n-1)& ~0xFFFULL;

    for(uint64_t p =first_page ; p<=last_page;p +=0x1000){
        map_pages(p, pmm_alloc_zero(), 1, P_WRITE);

    }
    void *ptr = (void *)heap_cur;
    heap_cur +=n;
    return ptr;
}
 void kfree(void * p){
     kmemset(p,'\0',sizeof(*p));
}
