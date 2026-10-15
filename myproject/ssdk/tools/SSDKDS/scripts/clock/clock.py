#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import sys
import project
sys.path.append(os.path.dirname(__file__))

from file_ops import FileBase
from excel_ops import WorkBookRead, SheetRead
from clock_node import clk_core_list, clk_node_list, gating_node_list
from ckgen_res import PllSheet, CkgenSheet, UUUSheet, GatingSheet

kunlun_driver_head = \
"""
#include <driver/clk/sc_pfpll_drv.h>
#include <driver/clk/kunlun_ckgen_drv.h>"""

taishan_driver_head = \
"""
#include <driver/clk/taishan_pll_drv.h>
#include <driver/clk/taishan_ckgen_drv.h>"""


# for clock_cfg.h file
class ClockIncFile(FileBase):
    prefix = FileBase.STATEMENT

    clk_header = """
#ifndef %s
#define %s

#ifdef CONFIG_CLK
%s
"""

    suffix = """

#endif

#endif
"""
    def get_clock_header(self, clock_header):
        filename = self.filename.split('\\')[-1]
        macro = filename.replace('.', '_').upper()
        return self.clk_header % (macro, macro, clock_header)


# for clock_cfg.c file
class ClockSrcFile(FileBase):
    prefix = FileBase.STATEMENT + """
#ifdef CONFIG_CLK

#include <types.h>
#include <param.h>
#include <regs_base.h>

#include "clock_cfg.h"
"""

    suffix = """
#endif"""

# for clock_cfg.h file
class TaishanClockIncFile(FileBase):
    prefix = FileBase.STATEMENT + """
#ifndef CLOCK_CFG_H
#define CLOCK_CFG_H

#ifdef CONFIG_CLOCK_OVER_DRIVE_MODE
#include "clock_cfg_od.h"
#else
#include "clock_cfg_nd.h"
#endif

#endif /* CLOCK_CFG_H */
"""


# for clock_cfg.h file
class TaishanClockSrcFile(FileBase):
    prefix = FileBase.STATEMENT + """
#ifdef CONFIG_CLK

#ifdef CONFIG_CLOCK_OVER_DRIVE_MODE
#include "clock_cfg_od.c"
#else
#include "clock_cfg_nd.c"
#endif

#endif
"""

def wirte_clk_list(clock_src, clock_inc):
    core_iter = clk_core_list.get_list_iter()
    for node in core_iter:
        node.write_struct(clock_src, clock_inc)

    node_iter = clk_node_list.get_list_iter()
    for node in node_iter:
        node.write_struct(clock_src, clock_inc)

    gating_iter = gating_node_list.get_list_iter()
    for node in gating_iter:
        node.write_struct(clock_src, clock_inc)


def get_prepare_list():
    prepare_list = []
    for node in clk_node_list.get_list_iter():
        if node.get_prepare() and node.core is not None:
            if 'bus_saf_plat' in node.data_name:
                prepare_list.insert(0, node)
            else:
                prepare_list.append(node)
    return prepare_list


def write_prepare_list(clock_src):
    prepare_list = get_prepare_list()
    if len(prepare_list) > 0:
        clock_src.write(
"""
const clk_node_t *board_clk_prepare_array[] = {""")
        for node in prepare_list:
            clock_src.write(
"""
    CLK_NODE(%s),""" % (node.data_name))
        clock_src.write(
"""
};
""")
    return len(prepare_list) > 0


def write_enable_list(clock_src):
    global enable_num
    enable_num = 0

    gating_iter = gating_node_list.get_list_iter()
    for node in gating_iter:
        if node.get_enable() == 1:
            if enable_num == 0:
                clock_src.write(
"""
const clk_node_t *board_clk_enable_array[] = {""")

            enable_num += 1
            clock_src.write(
"""
    CLK_NODE(%s),""" % (node.data_name))

    if enable_num > 0:
        clock_src.write(
"""
};
""")
    return enable_num > 0


def write_disable_list(clock_src):
    global disable_num
    disable_num = 0

    gating_iter = gating_node_list.get_list_iter()
    for node in gating_iter:
        if node.get_enable() == 2:
            if disable_num == 0:
                clock_src.write(
"""
const clk_node_t *board_clk_disable_array[] = {""")

            disable_num += 1
            clock_src.write(
"""
    CLK_NODE(%s),""" % (node.data_name))

    if disable_num > 0:
        clock_src.write(
"""
};
""")
    return disable_num > 0


def write_clk_init(clock_src, clock_inc):

    prepare_en = write_prepare_list(clock_src)
    enable_en = write_enable_list(clock_src)
    disable_en = write_disable_list(clock_src)

    clock_src.write(
"""
void board_clk_init(void)
{""")

    if disable_en:
        clock_src.write(
"""
    /* Disable unused clock. */
    for (uint32_t i = 0; i < ARRAY_SIZE(board_clk_disable_array); i++) {
        clk_disable((clk_node_t*)board_clk_disable_array[i]);
    }""")

    if enable_en:
        clock_src.write(
"""
    /* Enable used clock. */
    for (uint32_t i = 0; i < ARRAY_SIZE(board_clk_enable_array); i++) {
        clk_enable((clk_node_t*)board_clk_enable_array[i]);
    }""")

    if prepare_en:
        clock_src.write(
"""
    /* Prepare clock tree. */
    for (uint32_t i = 0; i < ARRAY_SIZE(board_clk_prepare_array); i++) {
        clk_prepare((clk_node_t*)board_clk_prepare_array[i]);
    }""")
    clock_src.write("""
}
""")

    clock_inc.write(
"""

extern void board_clk_init(void);
""")


def clk_list_init():
    clk_core_list.clear()
    clk_node_list.clear()
    gating_node_list.clear()

def clock_cfg_gen(project, workbook, domain_list, suffix):

    clk_list_init()

    for domain in domain_list:
        if suffix is None:
            pll_name = 'PLL2CKGEN_%s' % domain
            ckgen_name = 'CKGEN_%s' % domain
            clock_cfg_name = 'clock_cfg'
        else:
            pll_name = 'PLL2CKGEN_%s_%s' % (domain, suffix)
            ckgen_name = 'CKGEN_%s_%s' % (domain, suffix)
            clock_cfg_name = 'clock_cfg_%s' % suffix.lower()

        pll_sheet = PllSheet(workbook, pll_name, domain)
        pll_sheet.gen_res(0, project.platform)

        ckgen_sheet = CkgenSheet(workbook, ckgen_name, pll_sheet, domain)
        ckgen_sheet.gen_res(2, project.platform)

        if domain == "SOC":
            uuu_sheet = UUUSheet(workbook, "UUU", ckgen_sheet)
            uuu_sheet.gen_res(0, project.platform)

        gating_sheet = GatingSheet(workbook, "GATING_" + domain, domain)
        gating_sheet.gen_res(0, project.platform)

    with ClockSrcFile(project.get_generated_path('%s.c' % clock_cfg_name)) as clock_src:
        with ClockIncFile(project.get_generated_path('%s.h' % clock_cfg_name)) as clock_inc:
            if project.platform == 'kunlun':
                clock_header = kunlun_driver_head
            else:
                clock_header = taishan_driver_head
            clock_inc.write(clock_inc.get_clock_header(clock_header))
            wirte_clk_list(clock_src, clock_inc)
            write_clk_init(clock_src, clock_inc)


def taishan_clock_cfg_gen(project):
    clock_inc = TaishanClockIncFile(project.get_generated_path('clock_cfg.h'))
    clock_src = TaishanClockSrcFile(project.get_generated_path('clock_cfg.c'))

def gen(project):

    workbook = WorkBookRead(project.get_config_path('clk.dat'))

    if project.platform == 'kunlun':
        domain_list = ["SAF", "SEC", "DISP", "SOC"]
        clock_cfg_gen(project, workbook, domain_list, None)
    elif project.platform == 'taishan':
        domain_list = ["SF", "AP"]
        if workbook.check_sheet_exist('PLL2CKGEN_SF_ND'):
            clock_cfg_gen(project, workbook, domain_list, 'OD')
            clock_cfg_gen(project, workbook, domain_list, 'ND')
            taishan_clock_cfg_gen(project)
        else:
            clock_cfg_gen(project, workbook, domain_list, None)
    else:
        raise Exception('Unknown platform {}!' .format(project.paltfrom))

