/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _INCLUDE_REG_H_
#define _INCLUDE_REG_H_

#include <stdint.h>
#include <types.h>

#define REG64(addr) ((volatile uint64_t *)(uintptr_t)(addr))
#define REG32(addr) ((volatile uint32_t *)(uintptr_t)(addr))
#define REG16(addr) ((volatile uint16_t *)(uintptr_t)(addr))
#define REG8(addr) ((volatile uint8_t *)(uintptr_t)(addr))

#define RMWREG64(addr, startbit, width, val) *REG64(addr) = (*REG64(addr) & ~((((uint64_t)1u<<(width)) - 1u) << (startbit))) | ((uint64_t)(val) << (startbit))
#define RMWREG32(addr, startbit, width, val) *REG32(addr) = (*REG32(addr) & ~((((uint32_t)1u<<(width)) - 1u) << (startbit))) | ((uint32_t)(val) << (startbit))
#define RMWREG16(addr, startbit, width, val) *REG16(addr) = (*REG16(addr) & ~((((uint16_t)1u<<(width)) - 1u) << (startbit))) | ((uint16_t)(val) << (startbit))
#define RMWREG8(addr, startbit, width, val) *REG8(addr) = (*REG8(addr) & ~((((uint8_t)1u<<(width)) - 1u) << (startbit))) | ((uint8_t)(val) << (startbit))

#define writeq(v, a) (*REG64(a) = (v))
#define readq(a) (*REG64(a))
#define writel(v, a) (*REG32(a) = (v))
#define readl(a) (*REG32(a))
#define writew(v, a) (*REG16(a) = (v))
#define readw(a) (*REG16(a))
#define writeb(v, a) (*REG8(a) = (v))
#define readb(a) (*REG8(a))

static inline
void clrbits_32(volatile uint32_t *addr, uint32_t clear)
{
    *addr &= ~clear;
}

static inline
void setbits_32(volatile uint32_t *addr, uint32_t set)
{
    *addr |= set;
}

static inline
void clrsetbits_32(volatile uint32_t *addr, uint32_t clear, uint32_t set)
{
    uint32_t temp;

    temp = *addr;
    temp &= ~clear;
    temp |= set;
    *addr = temp;
}

#define CLRBITS_32(addr, clear)         clrbits_32(REG32(addr), clear)
#define SETBITS_32(addr, set)           setbits_32(REG32(addr), set)
#define CLRSETBITS_32(addr, clear, set) clrsetbits_32(REG32(addr), clear, set)

/* Optimized copy functions to read from/write to IO sapce */
#ifdef CONFIG_ARCH_ARM64

#define IS_ALIGNED(a, b) (!(((uintptr_t)(a)) & (((uintptr_t)(b))-1)))
/*
 * Copy data from IO memory space to "real" memory space.
 */
static inline
void __memcpy_fromio(void *to, const volatile void *from, size_t count)
{
	while (count && !IS_ALIGNED((unsigned long)from, 8)) {
		*(uint8_t *)to = readb(from);
		from++;
		to++;
		count--;
	}

	while (count >= 8) {
		*(uint64_t *)to = readq(from);
		from += 8;
		to += 8;
		count -= 8;
	}

	while (count) {
		*(uint8_t *)to = readb(from);
		from++;
		to++;
		count--;
	}
}

/*
 * Copy data from "real" memory space to IO memory space.
 */
static inline
void __memcpy_toio(volatile void *to, const void *from, size_t count)
{
	while (count && !IS_ALIGNED((unsigned long)to, 8)) {
		writeb(*(uint8_t *)from, to);
		from++;
		to++;
		count--;
	}

	while (count >= 8) {
		writeq(*(uint64_t *)from, to);
		from += 8;
		to += 8;
		count -= 8;
	}

	while (count) {
		writeb(*(uint8_t *)from, to);
		from++;
		to++;
		count--;
	}
}

/*
 * "memset" on IO memory space.
 */
static inline
void __memset_io(volatile void *dst, int c, size_t count)
{
	uint64_t qc = (uint8_t)c;

	qc |= qc << 8;
	qc |= qc << 16;
	qc |= qc << 32;

	while (count && !IS_ALIGNED((unsigned long)dst, 8)) {
		writeb(c, dst);
		dst++;
		count--;
	}

	while (count >= 8) {
		writeq(qc, dst);
		dst += 8;
		count -= 8;
	}

	while (count) {
		writeb(c, dst);
		dst++;
		count--;
	}
}

#endif /* CONFIG_ARCH_ARM64 */

#ifdef CONFIG_ARCH_ARM64
#define memset_io(a, b, c)		__memset_io((a), (b), (c))
#define memcpy_fromio(a, b, c)		__memcpy_fromio((a), (b), (c))
#define memcpy_toio(a, b, c)		__memcpy_toio((a), (b), (c))
#else
#define memset_io(a, b, c)		memset((void *)(a), (b), (c))
#define memcpy_fromio(a, b, c)		memcpy((a), (void *)(b), (c))
#define memcpy_toio(a, b, c)		memcpy((void *)(a), (b), (c))
#endif
#endif
