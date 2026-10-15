/*
 * Copyright (c) 2020 Semidrive Inc.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <types.h>
#include <bits.h>
#include <stdio.h>
#include <regs_base.h>
#include <reg.h>

#include <debug.h>

//#define DGB 1

#ifndef p2v
#define p2v
#endif

/* HPI QOS config */
static void init_qos_by_module(addr_t iobase, int offset,
                        uint32_t pri, uint32_t mode,
                        uint32_t bw, uint32_t satur, uint32_t ext)
{
	/* priority */
	writel(pri, iobase + offset + 0x8);
	/* mode */
	writel(mode, iobase + offset + 0xc);
	/* bw */
	writel(bw, iobase + offset + 0x10);
	/* saturation */
	writel(satur, iobase + offset + 0x14);
	/* ext */
	writel(ext, iobase + offset + 0x18);

#ifdef DGB
	ssdk_printf(SSDK_ALERT,
			"Addr: 0x%08x, priority:0x%08x, mode:0x%08x, bw:0x%08x, satur:0x%08x, ext:0x%x\n",
			(iobase + offset),
			readl(iobase + offset + 0x8),
			readl(iobase + offset + 0xc),
			readl(iobase + offset + 0x10),
			readl(iobase + offset + 0x14),
			readl(iobase + offset + 0x18));
#endif
}

void hpi_qos_init(void)
{
	addr_t iobase = 0;

	/* safety core resources */
	iobase = p2v(APB_GPV_SAF_BASE); //0xf12d_0000
	/* cr5_saf.prim */
	init_qos_by_module(iobase, 0x100, 0x80000704, 0x00000003, 0x000000b8, 0x00000008, 0x0);
	/* cr5_saf.perp */
	init_qos_by_module(iobase, 0x180, 0x80000704, 0x00000003, 0x000000b8, 0x00000008, 0x0);
	/* ce1 */
	init_qos_by_module(iobase, 0x200, 0x80000403, 0x00000003, 0x000000b8, 0x00000010, 0x0);
	/* dma1 */
	init_qos_by_module(iobase, 0x280, 0x80000503, 0x00000003, 0x000000b8, 0x00000010, 0x0);
	/* enet1 */
	init_qos_by_module(iobase, 0x300, 0x80000503, 0x00000003, 0x00000099, 0x00000008, 0x0);

	/* DMA9 */
	init_qos_by_module(iobase, 0x380, 0x80000503, 0x00000003, 0x000000b8, 0x00000010, 0x0);
	/* XSPI_SLV */
	init_qos_by_module(iobase, 0x400, 0x80000603, 0x00000003, 0x000000f5, 0x00000010, 0x0);
}
