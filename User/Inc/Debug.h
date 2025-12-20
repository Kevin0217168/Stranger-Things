#ifndef DEBUG_MINI_H
#define DEBUG_MINI_H

// 编译时开关，0=关闭，1=开启
#ifndef DEBUG_ENABLE
#define DEBUG_ENABLE 0
#endif

// 极简printf，只支持 %d %x %s %c
void mini_print(const char *fmt, ...);

// 条件编译的调试宏
#if DEBUG_ENABLE
    #define DEBUG_LOG(fmt, ...) \
        do { \
            mini_print("%s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
            mini_print("\r\n"); \
        } while(0)
#else
    #define DEBUG_LOG(fmt, ...) ((void)0)
#endif

#endif