// icc_endian.h
#ifndef ICC_ENDIAN_H
#define ICC_ENDIAN_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int is_little_endian(void)
{
    const uint32_t x = 1;
    return *(const uint8_t*)&x == 1;
}

/* ============ 从 uint8_t 数组加载整数 ============ */

/**
 * @brief 从 buf 按 **大端序（Big-Endian）** 加载最多 8 字节为 uint64_t
 * @param buf: 输入数组（非空）
 * @param len: 字节数（1~8），超出部分视为0
 * @return 对应的整数值
 */
static inline uint64_t load_be(const uint8_t *buf, size_t len)
{
    if (!buf || len == 0) return 0;
    if (len > 8) len = 8;

    uint64_t val = 0;
    for (size_t i = 0; i < len; i++) {
        val = (val << 8) | buf[i];
    }
    return val;
}

/**
 * @brief 从 buf 按 **小端序（Little-Endian）** 加载最多 8 字节为 uint64_t
 */
static inline uint64_t load_le(const uint8_t *buf, size_t len)
{
    if (!buf || len == 0) return 0;
    if (len > 8) len = 8;

    uint64_t val = 0;
    for (size_t i = 0; i < len; i++) {
        val |= ((uint64_t)buf[i]) << (i * 8);
    }
    return val;
}

/* ============ 将整数存储到 uint8_t 数组 ============ */

/**
 * @brief 将 val 按 **大端序（Big-Endian）** 存入 buf 的前 len 字节
 * @param buf: 输出数组（非空）
 * @param val: 要存储的值
 * @param len: 字节数（1~8），高位自动截断
 */
static inline void store_be(uint8_t *buf, uint64_t val, size_t len)
{
    if (!buf || len == 0) return;
    if (len > 8) len = 8;

    for (size_t i = 0; i < len; i++) {
        buf[len - 1 - i] = (uint8_t)(val & 0xFF);
        val >>= 8;
    }
}

/**
 * @brief 将 val 按 **小端序（Little-Endian）** 存入 buf 的前 len 字节
 */
static inline void store_le(uint8_t *buf, uint64_t val, size_t len)
{
    if (!buf || len == 0) return;
    if (len > 8) len = 8;

    for (size_t i = 0; i < len; i++) {
        buf[i] = (uint8_t)(val & 0xFF);
        val >>= 8;
    }
}

/* ============ 常用快捷宏 ============ */

#define LOAD_BE16(buf) ((uint16_t)load_be((buf), 2))
#define LOAD_BE32(buf) ((uint32_t)load_be((buf), 4))
#define LOAD_BE64(buf) (load_be((buf), 8))

#define LOAD_LE16(buf) ((uint16_t)load_le((buf), 2))
#define LOAD_LE32(buf) ((uint32_t)load_le((buf), 4))
#define LOAD_LE64(buf) (load_le((buf), 8))

#define STORE_BE16(buf, val) store_be((buf), (uint64_t)(val), 2)
#define STORE_BE32(buf, val) store_be((buf), (uint64_t)(val), 4)
#define STORE_BE64(buf, val) store_be((buf), (uint64_t)(val), 8)

#define STORE_LE16(buf, val) store_le((buf), (uint64_t)(val), 2)
#define STORE_LE32(buf, val) store_le((buf), (uint64_t)(val), 4)
#define STORE_LE64(buf, val) store_le((buf), (uint64_t)(val), 8)

#ifdef __cplusplus
}
#endif

#endif // ICC_ENDIAN_H
