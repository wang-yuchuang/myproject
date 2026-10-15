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

    # sdrv timer config
    sheet_name_sdrv = 'SemiDrive'
    template_sdrv = [
        ('Name', None),
        ('SemiDrive Timer ID.', '"SDRV_TMR_A,SDRV_TMR_B,SDRV_TMR_C,SDRV_TMR_D"'),
        ('Timer NO.', '"TIMER1,TIMER2,TIMER3,TIMER4,TIMER5,TIMER6,TIMER7,TIMER8,"'),
        ('IRQ', '"On,Off"'),
        ('Clock Source', '"SDRV_TMR_HF_CLK,SDRV_TMR_AHF_CLK,SDRV_TMR_LF_CLK,SDRV_TMR_LP_CLK,SDRV_TMR_ERR_CLK"'),
        ('Clock Frequency', None),
        ('Clock Division', '"1,2,4"'),
        ('As Kernel Timer', '"No,Yes"'),
    ]

    # xli ttc timer config
    sheet_name_xli = 'Xilinx'
    template_xli = [
        ('Name', None),
        ('Timer NO.', '"TIMER1,TIMER2,TIMER3,TIMER4,TIMER5,TIMER6,TIMER7,TIMER8,"'),
        ('IRQ', '"On,Off"'),
        ('Clock Frequency', None),
        ('As Kernel Timer', '"No,Yes"'),
    ]

    # timer cfg for x9 safety
    with PyxlExcelWriter('cfg_x9_safety.xlsx') as writer:
        # default sdrv timer config for x9 safety ref
        default_sdrv_x9 = [
            ['kernel_timer', 'SDRV_TMR_A', 'TIMER2',
                'On', 'SDRV_TMR_LF_CLK', 24000000, 1, 'Yes'],
        ]
        writer.create_template_sheet(
            sheet_name_sdrv, template_sdrv, default_sdrv_x9)

        # default xli ttc config for x9 safety ref
        default_xli_x9 = [[]]
        writer.create_template_sheet(
            sheet_name_xli, template_xli, default_xli_x9)

    # timer cfg for g9 flashloader
    with PyxlExcelWriter('cfg_g9.xlsx') as writer:
        # default sdrv timer config for g9 flashloader
        default_sdrv_g9 = [
            ['kernel_timer', 'SDRV_TMR_A', 'TIMER2',
                'On', 'SDRV_TMR_LF_CLK', 24000000, 1, 'Yes'],
        ]
        writer.create_template_sheet(
            sheet_name_sdrv, template_sdrv, default_sdrv_g9)

        # default xli ttc config for g9 flashloader
        default_xli_g9 = [[]]
        writer.create_template_sheet(
            sheet_name_xli, template_xli, default_xli_g9)

    # ==========================================================================
    # PyxlExcelReader example
    # ==========================================================================

    with PyxlExcelReader('cfg_x9_safety.xlsx') as reader:
        data = reader.read_sheet_into_dict(sheet_name_sdrv)

        pp = pprint.PrettyPrinter(indent=4)
        pp.pprint(data)
