# -*- coding: utf-8 -*-

import sys
import xlrd
import json
import re
import os
from read_excel import ExcelBase
from enum import Enum


part_ip_list = [
    'CAN',
    'LIN',
    'ETH',
    'USB',
    'SPI',
    'I2S',
    'ADC',
    'ACMP',
    'ETMR',
    'EPWM',
    'WDT',
]
STATEMENT = """
#*****************************************************************************
#
# WARNING: Automatically generated file, don't modify anymore!!!
#
# Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
# Software License Agreement
#
#*****************************************************************************
"""

def main(argv):
    print("device part json file path :" +argv[0])  

    fuse_config_file = open(argv[0], "r")
    fuseMap = json.load(fuse_config_file)
    maxMap = fuseMap["E3640"]
    #print(maxMap)

    for ip in part_ip_list:
        for index in maxMap[ip]:
            print("#ifdef CONFIG_" + ip + str(index))
            print("printf(\"" + ip + str(index) + " enable \");")
            print("#endif")
if __name__ == '__main__':
    main(sys.argv[1:])
