#include "lib/stdio.h"

#include <stdarg.h>
#include "arch/x64/drivers/serial.h" /* write_serial */

/* float / 二进制 / %n 关掉：内核里用不上，关了还能少带一堆代码 */
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 1 /* %lld / %llu / %llx */
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS 1 /* %hd / %hhu */
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0 /* %n，永远别要 */
#define NANOPRINTF_USE_ALT_FORM_FLAG 1               /* %#x */
#define NANOPRINTF_USE_FLOAT_SINGLE_PRECISION 0

#define NANOPRINTF_IMPLEMENTATION
#include "nanoprintf/nanoprintf.h"

static void serial_putc(int c, void *ctx){  //底座
    (void)ctx; //用不上，避免编译器警告
    if (c == '\n'){
        write_serial('\r');
    }
    write_serial((char)c);

}
static int vprintf(const char *fmt, va_list ap, int newline) {
    int n=npf_vpprintf(&serial_putc, 0, fmt, ap); //使用nanoprintf库的vpprintf函数来格式化输出
    if(newline){  //是否需要换行
        serial_putc('\n',0);
        n++;
    }
    return n;
}

 int printf(const char *format, ...){
    va_list ap;
    int n;
    va_start(ap,format);
    n=vprintf(format,ap,0);
    va_end(ap);
    return n;


}
 int println(const char *format, ...){
    va_list ap;
    int n;
    va_start(ap,format);
    n=vprintf(format,ap,1);
    va_end(ap);
    return n;


}