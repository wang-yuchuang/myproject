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

    # uart config
    sheet_sdrv = 'SemiDrive'
    template_sdrv = [
        ('Name', None),
        ('UART NO.', '"UART1,UART2,UART3,UART4,UART5,UART6,UART7,UART8"'),
        ('IRQ', '"On,Off"'),
        ('Baudrate', '"9600,19200,38400,57600,115200"'),
        ('Clock Frequency', None),
        ('Data Bits', '"5,6,7,8"'),
        ('Stop Bits', '"0.5,1,1.5,2"'),
        ('Parity', '"No,Odd,Even"'),
        ('As Console UART', '"No,Yes"'),
    ]

    sheet_dw = 'DesignWare'
    template_dw = [
        ('Name', None),
        ('UART NO.', '"UART1,UART2,UART3,UART4,UART5,UART6,UART7,UART8"'),
        ('IRQ', '"On,Off"'),
        ('Baudrate', '"9600,19200,38400,57600,115200"'),
        ('Clock Frequency', None),
        ('Data Bits', '"5,6,7,8"'),
        ('Stop Bits', '"0.5,1,1.5,2"'),
        ('Parity', '"No,Odd,Even"'),
        ('As Console UART', '"No,Yes"'),
    ]

    sheet_xli = 'Xilinx'
    template_xli = [
        ('Name', None),
        ('UART NO.', '"UART1,UART2,UART3,UART4,UART5,UART6,UART7,UART8"'),
        ('IRQ', '"On,Off"'),
        ('Baudrate', '"9600,19200,38400,57600,115200"'),
        ('Clock Frequency', None),
        ('Data Bits', '"5,6,7,8"'),
        ('Stop Bits', '"0.5,1,1.5,2"'),
        ('Parity', '"No,Odd,Even"'),
        ('As Console UART', '"No,Yes"'),
    ]

    # cfg for x9 safety
    with PyxlExcelWriter('cfg_x9_safety.xlsx') as writer:
        # default for x9 safety ref
        default_sdrv = [[]]
        default_dw = [
            ['console_uart', 'UART3', 'On', 115200, 80000000, 8, 1, 'No', 'Yes'],
        ]
        default_xli = [[]]
        writer.create_template_sheet(sheet_sdrv, template_sdrv, default_sdrv)
        writer.create_template_sheet(sheet_dw, template_dw, default_dw)
        writer.create_template_sheet(sheet_xli, template_xli, default_xli)

    # cfg for g9
    with PyxlExcelWriter('cfg_g9.xlsx') as writer:
        # default for g9
        default_sdrv = [[]]
        default_dw = [
            ['console_uart', 'UART4', 'On', 115200, 80000000, 8, 1, 'No', 'Yes'],
        ]
        default_xli = [[]]
        writer.create_template_sheet(sheet_sdrv, template_sdrv, default_sdrv)
        writer.create_template_sheet(sheet_dw, template_dw, default_dw)
        writer.create_template_sheet(sheet_xli, template_xli, default_xli)

    # ==========================================================================
    # PyxlExcelReader example
    # ==========================================================================

    with PyxlExcelReader('cfg_x9_safety.xlsx') as reader:
        data = reader.read_sheet_into_dict(sheet_dw)

        pp = pprint.PrettyPrinter(indent=4)
        pp.pprint(data)
