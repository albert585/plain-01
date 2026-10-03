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
    return ptr;}
// }void *buddy_alloc(size_t size) {
//     // 1. 向上取整到 2 的幂，算出阶
//     int order = 0;
//     while ((1 << order) * PAGE_SIZE < size) order++;

//     // 2. 从 order 开始往上找，找第一个非空的链表
//     int current = order;
//     while (current <= MAX_ORDER && free_area[current] == NULL)
//         current++;

//     if (current > MAX_ORDER) return NULL;  // 没内存了

//     // 3. 从 current 阶取一个块，然后一路向下分裂到 order
//     FreeBlock *block = free_area[current];
//     free_area[current] = block->next;  // 从链表里摘掉

//     while (current > order) {
//         current--;
//         // 把当前块一分为二，后半块挂到 current 阶的空闲链表
//         FreeBlock *buddy = (FreeBlock *)((char *)block + (1 << current) * PAGE_SIZE);
//         buddy->next = free_area[current];
//         free_area[current] = buddy;
//         // block 保持在前半块，继续往下分裂
//     }

//     return block;
// }
// void kfree(void * p){//hei AI what should i do ,不要补全了写一点东西好不好 
//     // In a simple kernel memory allocator, kfree might not actually free memory.
//     // Instead, it could just mark the memory as available for future allocations.
//     // However, in this implementation, we are not keeping track of allocated blocks,
//     // so we cannot implement a proper free function. 
//     // A more advanced allocator would maintain a free list or use a buddy system.
//     //所以我要干嘛,copilot?
//     // You could implement a simple free list to keep track of allocated blocks and their sizes.
//     //what you mean? copilot?
//     // A free list is a data structure that keeps track of memory blocks that have been freed
//     // and are available for future allocations. When you call kfree, you would add the block
//     // to the free list, and when you call kmalloc, you would check the free list for available blocks before allocating new memory. This way, you can reuse memory that has been freed
//     //give me an example
//     // Sure! Here's a simple example of how you might implement a free list for your memory allocator. This is a basic implementation and can be improved further.
//     //where?
//     // You can implement the free list in your memory.c file. Below is a simple example of how you might do this:
//     // Define a structure for the free list node
//     typedef struct FreeBlock {
//         size_t size; // Size of the free block
//         struct FreeBlock *next; // Pointer to the next free block
//     } FreeBlock;
//     //then?
//     // You can maintain a global pointer to the head of the free list
//     FreeBlock *free_list_head = NULL;
//     //go on
//     // In your kfree function, you would add the freed block to the free list. Here's an example implementation of kfree:
//     void kfree(void *p) {
//         if (p == NULL) {
//             return; // Do nothing if the pointer is NULL        
//         }
//         // Create a new free block
//         FreeBlock *block = (FreeBlock *)p;
//         block->size = /* size of the block being freed, you need to track this */;
//         block->next = free_list_head; // Point to the current head of the free list
//         free_list_head = block; // Update the head of the free list to the new block
//         // Note: You need to keep track of the size of each allocated block when you allocate memory in kmalloc, so you can set the size correctly here.
//         //go
//         // In your kmalloc function, you would first check the free list for available blocks before allocating new memory. Here's an example implementation of kmalloc that checks the free list:
//         void *kmalloc(size_t n) {
//             n = (n + 15) & ~15ULL; // 16-byte alignment
//             // Check the free list for available blocks
//             FreeBlock *prev = NULL;
//             FreeBlock *current = free_list_head;
//             while (current != NULL) {
//                 if (current->size >= n) {
//                     // Found a suitable block
//                     if (prev == NULL) {
//                         free_list_head = current->next; // Remove from free list
//                     } else {
//                         prev->next = current->next; // Remove from free list
//                         //please dont stop
//                         // Return the block to the caller
//                         return (void *)current; 
//                         // Note: You may want to split the block if it's larger than needed, but this is a simple example.  
// }
void kfree(void){};