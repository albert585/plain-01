#ifndef PLAIN_LIB_PRINTF_H
#define PLAIN_LIB_PRINTF_H

/* 串口调试打印。格式化核心是 kernel/src/nanoprintf/nanoprintf.h（第三方，见 NOTICE），
 * 这里只加"输出到串口 0x3F8"这一层：
 *   printf(...)  —— 不补换行
 *   println(...) —— 末尾自动补一个 '\n'（串口上补成 "\r\n"，与 serial_printk 一致）
 *
 * 格式能力由 nanoprintf 决定：%d %u %x %X %o %p %s %c %%、宽度/精度/补零/左对齐
 * 都支持（"%08x"、"%-5d"、"%.3s"、"%%"）；已关闭浮点、二进制和 %n。
 *
 * 两个实测出来的坑（v0.8.1, x86_64 LP64，逐条与 libc snprintf 对比过）：
 *   - %lu / %ld / %llu / %zu 都能用，输出和 libc 一致，所以 uint64_t 直接 %lu 就行，
 *     不用强转（上游头文件里"lack of 'l' length modifier support"那句注释已经过时）。
 *   - %p 是唯一和 libc 不同的：它输出不带 "0x" 前缀、按 2*sizeof(void*) 定宽补零的
 *     十六进制（"00000000deadbeef"）。想要 0x 前缀就自己写 "0x%llx" 加强转。 */

int printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
int println(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

#endif /* PLAIN_LIB_PRINTF_H */
