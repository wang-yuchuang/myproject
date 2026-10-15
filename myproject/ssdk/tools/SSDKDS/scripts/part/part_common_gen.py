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
def write_invisible_section(file,part,support,periph):
    periph=periph.replace("LIN","UART")
    file.write("config " + periph + "\r\n")
    file.write("	bool" + "\r\n")
    if(support):
        file.write("	default y if " + part + "\r\n")
    else:
        file.write("	default n if " + part + "\r\n")

def write_string_section(file,part,name,data):
    name=name.replace("LIN","UART")
    file.write("config " + name + "\r\n")
    file.write("	string" + "\r\n")
    file.write("	default " + "\"" + str(data) + "\"" + " if " + part + "\r\n")

def write_hex_section(file,part,name,data):
    name=name.replace("LIN","UART")
    file.write("config " + name + "\r\n")
    file.write("	hex" + "\r\n")
    file.write("	default "  + str(data) + " if " + part + "\r\n")

def write_bool_section(file,part,name,data):
    name=name.replace("LIN","UART")
    file.write("config " + name + "\r\n")
    file.write("	bool" + "\r\n")
    if(data):
        file.write("	default "  + "y" + " if " + part + "\r\n")
    else:
        file.write("	default "  + "n" + " if " + part + "\r\n")

def main(argv):
    print("board json file path :" + argv[0])
    print("device part json file path :" +argv[1])

    board_file = open(argv[0], "r")
    boardMap = json.load(board_file)["board_support"]

    fuse_config_file = open(argv[1], "r")
    fuseMap = json.load(fuse_config_file)

    """ generate kconfig file """
    outPath = os.getcwd() + "/part"
    print("kconfig out dir is :" + outPath)

    if(os.path.exists(outPath)):
        pass
    else:
        print("create output dir\r\n")
        os.mkdir(outPath)

    # create common kconfig entry
    with open(outPath +"/Kconfig", 'w') as f:
            f.write(STATEMENT)
            f.write("choice \r\n")
            f.write("	prompt \"Select Core ID\"\r\n")
            f.write("	default E3640\r\n")

            for part in fuseMap.keys():
                if(part == "reserve"):
                    continue

                f.write("		config " + part + "\r\n")
                f.write("		bool " + "\"" + part + "\"" + "\r\n")
                f.write("		depends on PACKAGE_" + fuseMap[part]["PACKAGE"] + "\r\n")
            f.write("endchoice\r\n\r\n")

            for part in fuseMap.keys():
                if(part == "reserve"):
                    continue            
                f.write("rsource " + "\"" + part +".Kconfig" + "\"" +"\r\n")
    f.close()

    for part in fuseMap.keys():
        if(part == "reserve"):
            continue           
        file = open(outPath + "/" + part +".Kconfig", 'w')
        detailDict = fuseMap[part]
        #print(detailDict)
        #write_invisible_section(f,part,)
        for ip in part_ip_list:
            if(detailDict[ip + "_NUM"] == 0):
                continue
            for index in detailDict[ip]:
                periphName = "PART_" + ip + str(index)
                write_invisible_section(file,part,1,periphName)
        
        ram_size_limit_str = detailDict["RAM_SIZE"]
        if ("MB" in ram_size_limit_str):
            ram_size = ram_size_limit_str.replace("MB","")
            #print(ram_size)
            ram_size_int = int(ram_size) * 1024 * 1024
            #print(type(ram_size_int))
            #print('%#x'%ram_size_int)
        elif("KB" in ram_size_limit_str):
            ram_size = ram_size_limit_str.replace("KB","")
            #print(ram_size)
            ram_size_int = int(ram_size) * 1024
            #print(type(ram_size_int))
            #print('%#x'%ram_size_int)
        write_hex_section(file,part,"IRAM_LIMIT",'%#x'%ram_size_int)
        part_hex_str = re.findall(r'\d*',part)
        
        part_hex_int = int(part_hex_str[1],16)
        #print('%#x'%part_hex_int)
        write_hex_section(file,part,"PART_ID",'%#x'%part_hex_int)

        # core support
        write_bool_section(file,part,"PART_SF",detailDict["CONFIG_SF"])
        write_bool_section(file,part,"PART_SX",detailDict["CONFIG_SX"])
        write_bool_section(file,part,"PART_SP",detailDict["CONFIG_SP"])

        write_bool_section(file,part,"PART_SF_LS_DISABLE",detailDict["CONFIG_SF_LS_DISABLE"])
        write_bool_section(file,part,"PART_SX_LS_DISABLE",detailDict["CONFIG_SX_LS_DISABLE"])
        write_bool_section(file,part,"PART_SP_LS_DISABLE",detailDict["CONFIG_SP_LS_DISABLE"])
        
        #special peripheral support
        write_bool_section(file,part,"PART_CSI",detailDict["CSI_NUM"])
        write_bool_section(file,part,"PART_G2D",detailDict["G2D_NUM"])
        write_bool_section(file,part,"PART_LVDS",detailDict["LVDS_NUM"])
        write_bool_section(file,part,"PART_RGB",detailDict["RGB_NUM"])
        write_bool_section(file,part,"PART_SDRAM",detailDict["SDRAM_NUM"])
        write_bool_section(file,part,"PART_GAMA",detailDict["GAMA_NUM"])
        write_bool_section(file,part,"PART_SDIO_SEHC2",detailDict["SDIO_SEHC2"])
        write_bool_section(file,part,"PART_SDIO_SEHC1",detailDict["SDIO_SEHC1"])

        #normal peripheral support
        write_hex_section(file,part,"PART_CAN_NUM",'%#x'%int(detailDict["CAN_NUM"]))
        write_hex_section(file,part,"PART_LIN_NUM",'%#x'%int(detailDict["LIN_NUM"]))
        write_hex_section(file,part,"PART_ETH_NUM",'%#x'%int(detailDict["ETH_NUM"]))
        write_hex_section(file,part,"PART_USB_NUM",'%#x'%int(detailDict["USB_NUM"]))
        write_hex_section(file,part,"PART_SPI_NUM",'%#x'%int(detailDict["SPI_NUM"]))
        write_hex_section(file,part,"PART_I2S_NUM",'%#x'%int(detailDict["I2S_NUM"]))
        write_hex_section(file,part,"PART_AIN_NUM",'%#x'%int(detailDict["AIN_NUM"]))
        write_hex_section(file,part,"PART_ADC_NUM",'%#x'%int(detailDict["ADC_NUM"]))
        write_hex_section(file,part,"PART_ACMP_NUM",'%#x'%int(detailDict["ACMP_NUM"]))

        #max frequency
        freq_MHZ = int(detailDict["MAX_FREQ"])
        freq_HZ = freq_MHZ * 1000 * 1000
        write_hex_section(file,part,"MAX_FREQ",'%#x'%freq_HZ)
        file.close()
if __name__ == '__main__':
    main(sys.argv[1:])
