#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import argparse
import sys

sys.path.append(os.path.join(os.getcwd(), 'tools/SSDKDS/scripts'))
from Kconfiglib.sdkconfig import SDKconfig

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='manual to this script')
    parser.add_argument("--kconfig_path", type=str, default="0")
    parser.add_argument("--defconfig_path", type=str, default="0")
    parser.add_argument("--out_path", type=str, default="0")
    args = parser.parse_args()

    print(args.defconfig_path)
    print(args.kconfig_path)
    print(args.out_path)

    # Kconfig
    kconf = SDKconfig(args.defconfig_path, args.kconfig_path, args.out_path)
    kconf.menuconfig()
