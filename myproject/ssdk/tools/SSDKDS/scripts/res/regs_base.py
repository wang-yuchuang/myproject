#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import sys
sys.path.append(os.path.dirname(__file__))

from file_ops import FileBase
from excel_ops import WorkBookRead, SheetRead
import project

# for regs_base.h file
class RegsIncFile(FileBase):
    prefix = FileBase.STATEMENT + """
#ifndef REGS_BASE_H
#define REGS_BASE_H

"""

    suffix = """

#endif /* REGS_BASE_H */
"""

class MemSheet(SheetRead):

    col_info = {
        "Name":             -1,
        "Type":             -1,
        "Start Address":    -1,
        "Resource Valid":   -1
    }

    def __init__(self, workbook, sheet_name):
        super().__init__(workbook, sheet_name)

    def preprocessing(self, start_row):
        super().init_col_index_info(start_row)
        self.load_res_list(start_row + 1)

    def gen_res(self, start_row, out_file):

        self.preprocessing(start_row)

        for item in self.res_list:
            type = item["Type"]
            if type == "PPC" or type == "RPC":
                name = "APB_%s_BASE" % (item["Name"].strip())
            else:
                name = "%s_BASE" % (item["Name"].strip())

            code = "#define {: <60} ({})\n".format(name, item["Start Address"])
            out_file.write(code)

def gen(project):

    workbook = WorkBookRead(project.get_config_path('mem.dat'))
    with RegsIncFile(project.get_generated_path('regs_base.h')) as regs_file:
        mem_sheet = MemSheet(workbook, "mem_map")
        mem_sheet.gen_res(0, regs_file)

