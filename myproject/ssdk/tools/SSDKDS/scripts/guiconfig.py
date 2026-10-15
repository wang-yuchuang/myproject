#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import argparse
import sys

PROJECT_PATH = os.getcwd()
SSDK_PATH = os.path.join(PROJECT_PATH, '../../')
SSDKDS_PATH = os.path.join(SSDK_PATH, 'tools/SSDKDS/scripts')

sys.path.append(SSDKDS_PATH)
from Kconfiglib.sdkconfig import SDKconfig

if __name__ == "__main__":

    # Kconfig
    kconf = SDKconfig(os.getcwd())
    kconf.guiconfig()
