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
class part_device_sp_info(Enum):
    # Special when read 0 means 0 ,1 means _1  ,2 means _1 _2 
    ETH_SP = 33
    XSPI_SP = 34
    USB_SP = 35
    FLEXRAY_SP = 70
class part_device_info(Enum):

    #if read excel cell 1 means _1_2 index is enable 
    CAN_1_2 = 18
    CAN_3_4 = 19
    CAN_5_6 = 20
    CAN_7 = 21
    CAN_8_9_10_11_12_13_14_15 = 22
    CAN_16 = 23
    CAN_17_18_19_20 = 24
    CAN_21 = 25
    CAN_23 = 26
    CAN_22_24 = 27

    LIN_1_2_3_4 = 29
    LIN_5_6_7_8 = 30
    LIN_9_10_11_12 = 31
    LIN_13_14_15_16 =32

    I2C_1_2 = 39
    I2C_3_4 = 40
    I2C_5_6 = 41
    I2C_7_8 = 42

    SPI_3_4 = 44
    SPI_2_6 = 45
    SPI_1_5 = 46
    SPI_7_8 = 47

    I2S_1 = 50
    I2S_2 = 49
    
    ADC_1_2 = 53
    ADC_3 = 54

    ACMP_1_2 = 56
    ACMP_3_4 = 57

    ETMR_1_2 = 65
    ETMR_3_4 = 66

    EPWM_1_2 = 68
    EPWM_3_4 = 69

    WDT_1_2 = 72
    WDT_8 = 73
    WDT_3_4_5_6 = 74

class part_normal_info(Enum):
    #common information
    RAM_SIZE = 10
    PACKAGE = 15
    CONFIG_IRAM1 = 11
    CONFIG_IRAM2 = 12
    CONFIG_IRAM3 = 13
    CONFIG_IRAM4 = 14
    CONFIG_SF = 4
    CONFIG_SF_LS_DISABLE = 5
    CONFIG_SX = 6
    CONFIG_SX_LS_DISABLE = 7
    CONFIG_SP = 8
    CONFIG_SP_LS_DISABLE = 9
    MAX_FREQ = 16
    CAN_NUM = 17
    LIN_NUM = 28
    ETH_NUM = 33
    XSPI_NUM = 34
    USB_NUM = 35
    SDIO_SEHC2 = 36
    SDIO_SEHC1 = 37
    I2C_NUM = 38
    SPI_NUM = 43
    I2S_NUM = 48
    AIN_NUM = 51
    ADC_NUM = 52
    ACMP_NUM = 55
    CSI_NUM = 58
    G2D_NUM = 59
    LVDS_NUM = 60
    RGB_NUM = 61
    SDRAM_NUM = 62
    GAMA_NUM = 63
    ETMR_NUM = 64
    EPWM_NUM = 67
    FLEXRAY_NUM = 70
    WDT_NUM = 71

def alter(file,old_str,new_str):
    file_data = ""
    with open(file, "r") as f:
        for line in f:
            if old_str in line:
                line = line.replace(old_str,new_str)
            file_data += line
    with open(file,"w") as f:
        f.write(file_data)

def main(argv):
    now_title = ""
    tmp = 0 
    data = 0

    book = xlrd.open_workbook(argv[0])
    fuse_table = book.sheet_by_name("Sheet1")

    nrows = fuse_table.nrows
    ncols = fuse_table.ncols

    #print(nrows)
    #print(ncols)

    with open(argv[1], 'w') as f:

        f.write("{\r\n")
        for row in range(nrows-1):
            tmp = 0
            deviceId = fuse_table.cell_value (rowx=row+1,colx=1)
            print(deviceId)
            f.write('"' + deviceId + '"')
            f.write(":{\r\n")
            for cols in part_normal_info:
                
                info = repr(cols)
                #print(info)
                cols_str = re.findall(r' \d*>',info)
                #print(cols_str)
                cols_str[0] = cols_str[0].replace('>','')
                cols_str[0] = cols_str[0].replace(' ','')
                #print(cols_str[0])
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
                #print(type(data))
                
                if(marco_str[0] == "RAM_SIZE" or marco_str[0] == "PACKAGE"):
                    str_tmp = "     " + '"' + marco_str[0] + '"'

                    f.write(str_tmp.ljust(30,' '))
                    f.write("     :      " + "\"" + str(data) + "\""+',\r\n') 
                else:
                    if(data !=""):
                        try:
                            data_int = int(data)
                        except:
                            data_str = re.findall(r'\d',data)
                            data_int = int(data_str[0])
                    else:
                        data_int = 0

                    str_tmp = "     " + '"' + marco_str[0] + '"'

                    f.write(str_tmp.ljust(30,' '))
                    f.write("     :      " + str(data_int) +',\r\n') 

            for cols in part_device_info:
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

                str_info = marco_str[0].split("_")
                
                if(now_title != str_info[0]):
                    if(tmp == 1):
                        f.write("END],")
                        f.write('\r\n')
                        tmp = 0
                    now_title = str_info[0]
                    str_tmp = "     " + '"' + now_title + '"'
                    f.write(str_tmp.ljust(30,' '))
                    f.write("     :      [")
                    tmp = 1

                data = fuse_table.cell_value (rowx=row+1,colx=cols-1)       
                if(data == 1.0):
                    for index in range(1,len(str_info)):
                        f.write(str_info[index])
                        f.write(",")
                data = 0

            f.write("END],")
            f.write('\r\n')

            for cols in part_device_sp_info:
                
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

                str_info = marco_str[0].split("_")

                data = fuse_table.cell_value (rowx=row+1,colx=cols-1)       
                
                conver_error = 0
                try :
                    data_int = int(data)
                except:
                    conver_error=1

                if(data != 0.0 and conver_error == 0):
                    now_title = str_info[0]
                    str_tmp = "     " + '"' + now_title + '"'
                    f.write(str_tmp.ljust(30,' '))
                    f.write("     :      [")
                    if(data == 1.0):
                        f.write("1,END],")
                        f.write("\r\n")
                    if(data == 2.0):
                        f.write("1,2,END],")
                        f.write("\r\n")
                    if(data == 3.0):
                        f.write("1,2,3,END],\r\n")
                        f.write("\r\n")
                if(conver_error == 1):
                        now_title = str_info[0]
                        str_tmp = "     " + '"' + now_title + '"'
                        f.write(str_tmp.ljust(30,' '))
                        f.write("     :      [")
                        f.write("1,END],")
                        f.write("\r\n")                    
                data = 0

            f.write("     " + '"' + "ARG" + '"' +"                         :      " + "0" +'\r\n') 
            f.write("},")
            f.write('\r\n') 
        f.write("\"reserve\"" + ":{}\r\n") 
        f.write('}\r\n')
    f.close()
if __name__ == '__main__':
    main(sys.argv[1:])
    alter(sys.argv[2], ",END", "")
    alter(sys.argv[2], "END", "")