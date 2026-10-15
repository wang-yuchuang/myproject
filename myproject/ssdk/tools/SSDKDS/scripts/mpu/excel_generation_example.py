#!/usr/bin/python3
# -*- coding: UTF-8 -*-

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from excel_ops import PyxlExcelReader, PyxlExcelWriter
import pprint  # for pretty print


if __name__ == '__main__':

    # ==========================================================================
    # PyxlExcelWriter example
    # ==========================================================================

    # mpu config
    sheet_name = 'MPU'
    template = [
        ('Address', None),
        ('Size', None),
        ('Type', '"StrongOrdered,Device,Normal,Normal Noncacheable,Normal RO,No Access"'),
        ('Comment', None),
    ]

    # cfg for x9 safety
    with PyxlExcelWriter('cfg_x9_safety.xlsx') as writer:
        # default for x9 safety ref
        default = [
            ['0', '0x40', 'Normal RO', None],
            ['MEMBASE', '0x40', 'Normal RO', None],
            ['0x7FFFE00', '0x200', 'Device', 'ospi1 indirect address space'],
            ['0x10000000', '0x10000000', 'Device', 'PCIe1 region address space'],
            ['0x20000000', '0x10000000', 'Device', 'PCIe2 region address space'],
        ]
        writer.create_template_sheet(sheet_name, template, default)

    # cfg for g9
    with PyxlExcelWriter('cfg_g9.xlsx') as writer:
        # default for g9
        default = [
            ['0', '0x40', 'Normal RO', None],
            ['MEMBASE', '0x40', 'Normal RO', None],
            ['0x7FFFE00', '0x200', 'Device', 'ospi1 indirect address space'],
            ['0x10000000', '0x10000000', 'Device', 'PCIe1 region address space'],
            ['0x20000000', '0x10000000', 'Device', 'PCIe2 region address space'],
        ]
        writer.create_template_sheet(sheet_name, template, default)

    # ==========================================================================
    # PyxlExcelReader example
    # ==========================================================================

    with PyxlExcelReader('cfg_x9_safety.xlsx') as reader:
        data = reader.read_sheet_into_dict(sheet_name)

        pp = pprint.PrettyPrinter(indent=4)
        pp.pprint(data)
