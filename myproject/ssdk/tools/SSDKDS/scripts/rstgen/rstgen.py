from file_ops import FileBase
from excel_ops import WorkBookRead, SheetRead
import project

kunlun_rstgen_head = \
"""
#include <driver/kunlun/reset/sdrv_reset_drv.h>"""

taishan_rstgen_head = \
"""
#include <driver/taishan/reset/taishan_reset_drv.h>"""

class RstgenDevSheet(SheetRead):

    col_info = {
        "Name"          :   -1,
        "self_rst_en"   :   -1,
        "sem_rst_en"    :   -1,
        "dbg_rst_en"    :   -1,
        "wdg1_rst_en"   :   -1,
        "wdg2_rst_en"   :   -1,
        "wdg3_rst_en"   :   -1,
        "wdg4_rst_en"   :   -1,
        "wdg5_rst_en"   :   -1,
        "wdg6_rst_en"   :   -1,
        "wdg7_rst_en"   :   -1,
        "wdg8_rst_en"   :   -1,
    }

    def __init__(self, workbook, sheet_name):
        super().__init__(workbook, sheet_name)
        super().init_col_index_info(0)
        super().load_res_list(1)


class RstgenModuleSheet(SheetRead):

    col_info = {
        "Name"      :   -1,
        "Type"      :   -1,
        "Domain"    :   -1,
        "Reset"     :   -1,
    }

    def __init__(self, workbook, sheet_name):
        super().__init__(workbook, sheet_name)
        super().init_col_index_info(0)
        super().load_res_list(1)


# for rstgen_cfg.h file
class RstgenIncFile(FileBase):
    prefix = FileBase.prefix + """
#ifndef BOARDS_RSTGEN_CFG_H
#define BOARDS_RSTGEN_CFG_H

#ifdef CONFIG_RESETGEN

#include <driver/reset.h>

"""

    suffix = """

extern void board_reset_init(void);

#endif

#endif /* BOARDS_RSTGEN_CFG_H */
"""

# for rstgen_cfg.c file
class RstgenSrcFile(FileBase):
    prefix = FileBase.prefix + """
#ifdef CONFIG_RESETGEN

#include <types.h>
#include <param.h>
"""

    rstgen_header = """
%s
#include <regs_base.h>

#include "rstgen_cfg.h"
"""

    suffix = """
#endif"""


class RstGenDev(object):

    def __init__(self, name):
        self.name = name
        self.init_func = 'sdrv_reset_init'
        self.dev_name = 'sdrv_rstgen_%s_dev' % (name.lower())
        self.rstgen_name = 'g_rstgen_%s' % (name.lower())

    def gen_init_func(self):
        return '%s(&%s, &%s)' % (self.init_func, self.rstgen_name, self.dev_name)

    def gen_rstgen_dev_code(self, data_dict):
        rstgen_dev_code = """
static sdrv_reset_ctl_t %s = {
    .base = APB_RSTGEN_%s_BASE,""" % (self.dev_name, self.name)
        for key in data_dict:
            if (data_dict[key] == 'enable'):
                rstgen_dev_code += """
    .%s = 1,""" % (key)
        rstgen_dev_code += """
};
static reset_ctl_t %s;
""" % (self.rstgen_name)
        return rstgen_dev_code


class TaishanRstGenDev(RstGenDev):

    def __init__(self, name):
        self.name = name
        self.init_func = 'taishan_reset_init'
        self.dev_name = 'taishan_rstgen_%s_dev' % (name.lower())
        self.rstgen_name = 'g_rstgen_%s' % (('SAF' if name == 'SF' else 'AP').lower())

    def gen_rstgen_dev_code(self, data_dict):
        rstgen_dev_code = """
static taishan_reset_ctl_t %s = {
    .base = APB_RSTGEN_%s_BASE,""" % (self.dev_name, self.name)
        for key in data_dict:
            if (data_dict[key] == 'enable'):
                rstgen_dev_code += """
    .%s = 1,""" % (key)
        rstgen_dev_code += """
};
static reset_ctl_t %s;
""" % (self.rstgen_name)
        return rstgen_dev_code


def handle_rstgen_dev(workbook, rstgen_src, platform):
    rstgen_dev_sheet = RstgenDevSheet(workbook, 'DEV')
    init_code = ''
    for item in rstgen_dev_sheet.res_list:
        if platform == 'kunlun':
            rstgen_dev = RstGenDev(item['Name'])
        elif platform == 'taishan':
            rstgen_dev = TaishanRstGenDev(item['Name'])

        rstgen_src.write(rstgen_dev.gen_rstgen_dev_code(item))
        init_code += """
    %s;
""" % (rstgen_dev.gen_init_func())
    return init_code


def handle_rstgen_module(workbook, rstgen_src, rstgen_inc):
    rstgen_module_sheet = RstgenModuleSheet(workbook, 'MODULE')
    board_reset_list = ''
    board_reset_list_code = ''
    init_code = ''
    for item in rstgen_module_sheet.res_list:
        name = str(item['Name'])
        type = str(item['Type'])
        domain = str(item['Domain'])
        reset_flag = str(item['Reset'])
        rstsig_name = 'rstsig_%s' % (name.lower())
        rstgen_src.write("""
reset_sig_t %s = {
    .rst_ctl = &g_rstgen_%s,
    .id      = %s,
};
""" % (rstsig_name, domain.lower(),
       'RSTSIG_%s_%s' % ('SAF' if domain == 'SAF' else 'AP', name)))

        rstgen_inc.write("""
extern reset_sig_t %s;""" % (rstsig_name))

        if reset_flag == 'Yes':
            board_reset_list += """
    &%s,""" % (rstsig_name)

    if len(board_reset_list) != 0:
        rstgen_src.write("""
static reset_sig_t *board_reset_array[] = {%s
};
""" % (board_reset_list))
        init_code = """
    for (uint32_t i = 0; i < ARRAY_SIZE(board_reset_array); i++) {
        reset_ctl_reset(board_reset_array[i]);
    }"""
    return init_code


def write_rstgen_init_code(rstgen_src, init_code):
    rstgen_src.write("""
void board_reset_init(void)
{
%s
}""" % (init_code))


def gen(project):
    workbook = WorkBookRead(project.get_config_path('rstgen.dat'))

    if project.platform == 'kunlun':
        rstgen_head = RstgenSrcFile.rstgen_header % (kunlun_rstgen_head)
    elif project.platform == 'taishan':
        rstgen_head = RstgenSrcFile.rstgen_header % (taishan_rstgen_head)
    else:
        raise Exception('Unknown platform {}!' .format(project.paltfrom))

    with RstgenSrcFile(project.get_generated_path('rstgen_cfg.c')) as rstgen_src:
        rstgen_src.write(rstgen_head)
        with RstgenIncFile(project.get_generated_path('rstgen_cfg.h')) as rstgen_inc:
            init_code = handle_rstgen_dev(workbook, rstgen_src, project.platform)
            init_code += handle_rstgen_module(workbook, rstgen_src, rstgen_inc)
            write_rstgen_init_code(rstgen_src, init_code)
