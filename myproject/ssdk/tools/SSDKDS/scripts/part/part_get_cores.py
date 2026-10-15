#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import argparse
import sys

sys.path.append(os.path.join(os.getcwd(), 'tools/SSDKDS/scripts'))
from Kconfiglib.sdkconfig import SDKconfig

core_lists = []

def main():
    parser = argparse.ArgumentParser(description='manual to this script')
    parser.add_argument("--kconfig_path", type=str, default="0")
    parser.add_argument("--defconfig_path", type=str, default="0")
    parser.add_argument("--out_path", type=str, default="0")
    args = parser.parse_args()

    # Kconfig
    kconf = SDKconfig(args.defconfig_path, args.kconfig_path, args.out_path)

    core_lists.append('sf')
    if kconf.get_symb('PART_SP') == 'y':
        if kconf.get_symb('SP_Split') == 'y':
            core_lists.append('sp0')
            core_lists.append('sp1')
        elif kconf.get_symb('SP_Lockstep') == 'y':
            core_lists.append('sp0')
    if kconf.get_symb('PART_SX') == 'y':
        if kconf.get_symb('SX_Split') == 'y':
            core_lists.append('sx0')
            core_lists.append('sx1')
        elif kconf.get_symb('SX_Lockstep') == 'y':
            core_lists.append('sx0')
    print(core_lists)

if __name__ == "__main__":
    main()