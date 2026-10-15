# -*- coding: utf-8 -*-

import sys
import xlrd

import re
from read_excel import ExcelBase
from enum import Enum


STATEMENT = """
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
"""

class part_info(Enum):
    CONFIG_IRAM1 = 11
    CONFIG_IRAM2 = 12
    CONFIG_IRAM3 = 13
    CONFIG_IRAM4 = 14
    CONFIG_CAN_NUM = 17
    CONFIG_CAN1_CAN2 = 18
    CONFIG_CAN3_CAN4 = 19
    CONFIG_CAN5_CAN6 = 20
    CONFIG_CAN7 = 21
    CONFIG_CAN8TO15 = 22
    CONFIG_CAN16 = 23
    CONFIG_CAN17TO20 = 24
    CONFIG_CAN21 = 25
    CONFIG_CAN23 = 26
    CONFIG_CAN22TO24 = 27
    CONFIG_LIN_NUM = 28
    CONFIG_LIN1TO4 = 29
    CONFIG_LIN5TO8 = 30
    CONFIG_LIN9TO12 = 31
    CONFIG_LIN13TO16 =32
    CONFIG_ETH_NUM = 33
    CONFIG_XSP_NUM = 34
    CONFIG_USB_NUM = 35
    CONFIG_SDIO_SEHC2 = 36
    CONFIG_SDIO_ECHC1 = 37
    CONFIG_I2C1_I2C2 = 39
    CONFIG_I2C3_I2C4 = 40
    CONFIG_I2C5_I2C6 = 41
    CONFIG_I2C7_I2C8 = 42
    CONFIG_SPI_NUM = 43
    CONFIG_SPI3_SPI4 = 44
    CONFIG_SPI2_SPI6 = 45
    CONFIG_SPI1_SPI5 = 46
    CONFIG_SPI7_SPI8 = 47
    CONFIG_I2S_NUM = 48
    CONFIG_I2S1 = 50
    CONFIG_I2S2 = 49
    CONFIG_AIN_NUM = 51
    CONFIG_ADC_NUM = 52
    CONFIG_ADC1_ADC2 = 53
    CONFIG_ADC3_ADC4 = 54
    CONFIG_ACMP_NUM = 55
    CONFIG_ACMP12 = 56
    CONFIG_ACMP34 = 57
    CONFIG_CSI_NUM = 58
    CONFIG_G2D_NUM = 59
    CONFIG_LVDS_NUM = 60
    CONFIG_RGB_NUM = 61
    CONFIG_SDRAM_NUM = 62
    CONFIG_GAMA_NUM = 63
    CONFIG_ETMR_NUM = 64
    CONFIG_ETMR1_ETMR2 = 65
    CONFIG_ETMR3_ETMR4 = 66
    CONFIG_EPWM1_EPWM2 = 68
    CONFIG_EPWM3_EPWM4 = 69
    CONFIG_FLEXRAY_NUM = 70
    CONFIG_WDT_NUM = 71
    CONFIG_WDT1_WDT2 = 72
    CONFIG_WDT8 = 73
    CONFIG_WDT3_WDT4_WDT5 = 74

def main(argv):
    book = xlrd.open_workbook(argv[0])
    fuse_table = book.sheet_by_name("Sheet1")

    nrows = fuse_table.nrows
    ncols = fuse_table.ncols

    print(nrows)
    print(ncols)

    with open(argv[1], 'w') as f:
        f.write(STATEMENT)
        f.write('#ifndef __DEVICE_PART__\r\n')
        f.write('#define __DEVICE_PART__\r\n\r\n')
        for row in range(nrows-1):
            deviceId = fuse_table.cell_value (rowx=row+1,colx=1)
            f.write('#ifdef CONFIG_' + deviceId +'\r\n')
            deviceIdNum = re.findall(r'[1-9]\d*',deviceId)
            print(deviceIdNum)
            str_temp =""
            str_temp = "    #define CONFIG_PART" 
            f.write(str_temp.ljust(30,' '))
            f.write("            0x" + deviceIdNum[0] + '\r\n')
            

            for cols in part_info:
                info = repr(cols)
                #print(info)
                cols_str = re.findall(r' \d\d>',info)
                cols_str[0] = cols_str[0].replace('>','')
                cols_str[0] = cols_str[0].replace(' ','')
                #print(cols_str[0])
                try:
                    cols = int(cols_str[0])
                except:
                    print("conver cols failed \r\n")

                marco_str = re.findall(r'\..*:',info)
                marco_str[0] = marco_str[0].replace('.','')
                marco_str[0] = marco_str[0].replace(':','')

                #print(marco_str)
                data = fuse_table.cell_value (rowx=row+1,colx=cols-1)

                #print(marco_str)
                #print(data)

                if("NUM" in marco_str[0] and data !=""):
                    try:
                        data_int = int(data)
                    except:
                        data_str = re.findall(r'\d',data)
                        data_int = int(data_str[0])

                    str_tmp = "    "+'#define '+ marco_str[0]

                    f.write(str_tmp.ljust(30,' '))
                    f.write("            " + str(data_int) +'\r\n') 
                else:
                    if(data == 1.0):
                        f.write("    "+'#define '+ marco_str[0] +'\r\n')
            f.write('#endif\r\n') 
        f.write('#endif\r\n')
    f.close()
if __name__ == '__main__':
    main(sys.argv[1:])