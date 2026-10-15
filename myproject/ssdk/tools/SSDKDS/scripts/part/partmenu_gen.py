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
] 

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
    print("board json file path :" + argv[0])
    print("device part json file path :" +argv[1])  

    fuse_config_file = open(argv[1], "r")
    fuseMap = json.load(fuse_config_file)
    
    board_config_file = open(argv[0], "r")
    boardSupportFileDict = json.load(board_config_file)
    boardSupport = boardSupportFileDict['board_support']

    """ generate kconfig file """
    outPath = os.getcwd()
    print("kconfig out file is :" + outPath +"/Kconfig") 
    with open(outPath +"/Kconfig", 'w') as f:
        f.write(STATEMENT)
        f.write("comment \"Chip ID Configuration & Peripheral Configuration\"\r\n")
        f.write("choice\r\n")
        f.write("	prompt \"board chip ID\"\r\n")
        f.write("	default E3640\r\n")

        for part in fuseMap.keys():
            if(part == "reserve"):
                continue

            f.write("		config " + part + "\r\n")
            f.write("			bool " + "\"" + part + "\"\r\n")
            f.write("			if " + part + "\r\n")
            #f.write("			rsource " + "\"" + part + "\"" + "\r\n")

            detailDict = fuseMap[part]

            f.write("			config " + part +"_CORE_ID\r\n")
            f.write("			int " + "\"" + part + "_CORE_ID\"\r\n")
            #print(detailDict['RAM_SIZE'].encode("utf-8"))
            core_id_list = re.findall(r'\d*',part)
            #print(core_id_list)
            f.write("			default " + "\"" +str(core_id_list[1]) +"\"\r\n\r\n" )

            f.write("			config " + part +"_CORE_IRAM_SIZE\r\n")
            f.write("			string " + "\"" + part + "_CORE_IRAM_SIZE\"\r\n")
            #print(detailDict['RAM_SIZE'].encode("utf-8"))
            f.write("			default " + "\"" + detailDict['RAM_SIZE'].encode("utf-8") +"\"\r\n\r\n" )

            f.write("			config " + part + "_CORE_FREQ\r\n")
            f.write("			string " + "\"" + part + "_CORE_FREQ\"\r\n")
            #print(detailDict['MAX_FREQ'])
            f.write("			default " + "\"" + str(detailDict['MAX_FREQ']) +"MHZ\"\r\n\r\n" )

            """ create periperal configure config
                if board limit periperal , available is intersection of board and fuse
            """
            # board enable and fuse enable ip index
            ip_available_set = {}
            # fuse enable but board not enable
            ip_costom_available_set = {}
            # board enable list
            board_available_set = {}
            # fuse enable list
            part_available_set = {}
            for ip in part_ip_list:
                if(detailDict[ip + "_NUM"] == 0):
                    continue

                part_available_set = set(detailDict[ip])
                        
                if (ip in board_ip_list):
                    board_available_set = set(boardSupport[ip])
                    ip_available_set = set.intersection(board_available_set,part_available_set)
                else:
                    ip_available_set = part_available_set
                        
                f.write("			menu \"" + ip  + " Custom Config\"\r\n" )
                        
                for available_id in ip_available_set:
                    f.write("			config " + ip  + str(available_id) + "\r\n" )   
                    f.write("				bool " + "\"" + ip  + str(available_id) +" Enable\"\r\n")  
                    f.write("				default y\r\n")
                        
                ip_costom_available_set = part_available_set.difference(ip_available_set)
                    #if(part == "E3640"):
                        #print(part)
                        #print(ip)
                        #print(ip_costom_available_set)
                for available_coustom_id in ip_costom_available_set:
                    f.write("			config " + ip  + str(available_coustom_id) + "\r\n" )   
                    f.write("				bool " + "\"" + ip  + str(available_coustom_id) +" Enable\"\r\n")  
                    f.write("				default n\r\n")

                f.write("			endmenu\r\n")
            f.write("			endif\r\n")
        f.write("endchoice\r\n")

    print("generate part file success\r\n")
if __name__ == '__main__':
    main(sys.argv[1:])
