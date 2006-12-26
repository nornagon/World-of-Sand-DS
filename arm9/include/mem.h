#ifndef MEM_H
#define MEM_H

#ifdef __cplusplus
extern "C" {
#endif
extern void memset32(void *dst, u32 val, u32 n);
extern void memset16(void *dst, u16 val, u32 n);
extern void memcpy32(void *dst, const void *src, u32 n);
extern void memcpy16(void *dst, const void *src, u32 n);
#ifdef __cplusplus
}
#endif

#endif // MEM_H
