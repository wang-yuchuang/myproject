# -*- coding: utf-8 -*-

import sys
import xlrd
import json
import re
import os
from read_excel import ExcelBase
from enum import Enum


board_ip_list = [
    'CAN',
    'LIN',
    'ETH',
    'USB',
    'XSPI',
    'SPI',
    'I2S',
    'I2C',
    'ADC',
    'ACMP',
    'ETMR',
    'EPWM',
    'WDT',
]

def write_board_section(file,periph,board_support):
    periph=periph.replace("LIN","UART")
    file.write("		config " + periph + "\r\n")
    file.write("		bool " + "\"" + periph + " Support"+ "\"" +"\r\n")
    file.write("		depends on PART_" + periph +"\r\n")
    if(board_support):
        file.write("		default y"+"\r\n")
    else:
        file.write("		default n"+"\r\n")
    #####Require for louliang
    if("I2S" in periph):
        PDM_STR=periph.replace("I2S","PDM")
        file.write("		config " + PDM_STR + "\r\n")
        file.write("		bool " + "\"" + PDM_STR + " Support"+ "\"" +"\r\n")
        file.write("		depends on PART_" + periph +"\r\n")
        if(board_support):
            file.write("		default y"+"\r\n")
        else:
            file.write("		default n"+"\r\n")

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
    print("board json file path :" + argv[0])
    print("device part json file path :" +argv[1])  
    print("output Kconfig Name :" + argv[2])

    fuse_config_file = open(argv[1], "r")
    fuseMap = json.load(fuse_config_file)
    
    board_file = open(argv[0], "r")
    boardMap = json.load(board_file)["board_support"]

    """ generate kconfig file """
    outPath = os.getcwd() + "/board"
    print("board kconfig out dir is :" + outPath) 

    if(os.path.exists(outPath)):
        pass
    else:
        print("create output dir\r\n")
        os.mkdir(outPath)
    
    # create common kconfig entry
    with open(outPath +"/Kconfig_" + argv[2], 'w') as f:
            f.write(STATEMENT)
            boardname = argv[0].replace(".json","")
            boardname = boardname.replace("board_xml/","")
            f.write("config BOARD_" + boardname.upper() + "\r\n")
            f.write("	bool\r\n")
            f.write("	depends on ARCH_CHIP_E3\r\n")
            f.write("	default y\r\n\r\n")
            f.write("rsource \"$(CORE)/Kconfig\"\r\n\r\n")

            f.write("menu \"Board Peripheral Configuration\"\r\n")
            for ip in board_ip_list:
                maxDic = fuseMap["E3640"]
                #print(maxDic[ip])
                #print(boardMap)
                f.write("	menu " + "\""  + ip + " Peripheral Configuration" + "\""+ "\r\n")

                for index in maxDic[ip]:
                    if(index in boardMap[ip]):
                        board_support = 1
                    else:
                        board_support = 0 
                    write_board_section(f,ip+str(index),board_support)
                
                f.write("	endmenu\r\n\r\n")
            f.write("endmenu\r\n")
            f.write("config PACKAGE_" + boardMap["PACKAGE"] +"\r\n")
            f.write("bool " + "\r\n")
            f.write("default y\r\n")

if __name__ == '__main__':
    main(sys.argv[1:])
