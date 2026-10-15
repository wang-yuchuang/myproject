# -*- coding: utf-8 -*-

import os
import sys
import shutil

sys.path.append(os.path.dirname(__file__))
from Kconfiglib.sdkconfig import SDKconfig
from const_mk import const_mk
from file_ops import FileBase
from xml_ops import XmlBase
from config import Config
from clock import clock
from irq import irq
from pinmux import chip_pin
from res import regs_base
from uart import uart
from timer import timer
from mpu import mpu
from rstgen import rstgen
from remap import remap

module_map = {
    'uart'      :   uart,
    'timer'     :   timer,
    'irq'       :   irq,
    'clock'     :   clock,
    'mpu'       :   mpu,
    'rstgen'    :   rstgen,
    'remap'     :   remap,
    'regs'      :   regs_base,
    'pin'       :   chip_pin
}

class MKFile(FileBase):

    prefix = """
"""

    suffix = """
"""

class Project(object):

    def __init__(self, prj_name):
        self.prj = prj_name
        self.modules = []
        self.prj_path = os.path.join(os.getenv('SSDK_ROOT'), 'boards', self.prj)
        self.prj_data_path = os.path.join(os.getenv('SSDK_ROOT'),
                                          'tools/SSDKDS/data/project', self.prj)
        if not os.path.exists(self.prj_path):
            os.makedirs(self.prj_path)

        # gcc specialized dir
        os.makedirs(os.path.join(self.prj_path, 'gcc/'), exist_ok=True)

        # iar specialized dir
        os.makedirs(os.path.join(self.prj_path, 'iar/'), exist_ok=True)

    def __getitem__(self, key):
        if key in self.__dict__.keys():
            return self.__dict__[key]
        else:
            raise Exception("%s not exit !" % key)

    def __setitem__(self, key, value):
        self.__dict__[key] = value

    def get_board_info(self):
        board_xml = XmlBase(os.path.join(self.prj_data_path, 'board_info.xml'))
        root = board_xml.get_root()
        self.board = board_xml.get_text(root, 'board')
        self.platform = board_xml.get_text(root, 'platform')
        self.core = board_xml.get_text(root, 'core')
        self.arch = board_xml.get_text(root, 'arch')
        self.gic = board_xml.get_text(root, 'gic')
        self.board_data_path = os.path.join(os.getenv('SSDK_ROOT'),
                                            'tools/SSDKDS/data/board', self.board)

    def get_config_path(self, file_name):
        # Get config file in project folder first
        path = os.path.join(self.prj_data_path, file_name)
        if os.path.isfile(path):
            return path
        path = os.path.join(self.board_data_path, file_name)
        if os.path.isfile(path):
            return path
        else:
            raise Exception('Unknown Config file {}!' .format(file_name))

    def get_generated_path(self, file_name):
        return os.path.join(self.prj_path, file_name)

    def prepare_project_gen(self):

        # Read Board Info
        self.get_board_info()

        # Board Kconfig
        self._cp2prj('Kconfig')

        # Board default defconfig
        self._cp2prj('defconfig')

        # Kconfig
        self.kconfig = SDKconfig(self.prj_path)
        self.kconfig.guiconfig()

        # gen regs config file
        regs_base.gen(self)

        # irq config file
        irq.gen(self)

        # board config file
        self._cp2prj('board.h')
        self._cp2prj('board.c')

        # gen main source file
        self._cp2prj('main.c')

        # GCC specialized files
        self._cp2prj('gcc/ram.ld', 'gcc/%s.ld' % self.prj)

        # IAR specialized files
        self._cp2prj('iar/ram.icf', 'iar/%s.icf' % self.prj)
        self._cp2prj('iar/debug.mac', 'iar/%s.mac' % self.prj)

    def gen_module_config(self, module):
        # Read Board Info
        self.get_board_info()
        module_map[module].gen(self)

    def gen_modules_config(self):

        #scr config file
        if self.kconfig.get_symb('SCR') != 'n':
            self._cp2prj('scr.dat', 'scr_hw.h')

        # mpu config file
        if self.kconfig.get_symb('ARCH_USE_MPU') != 'n':
            mpu.gen(self)
        if self.kconfig.get_symb('UART') != 'n':
            uart.gen(self)
        if self.kconfig.get_symb('TIMER') != 'n':
            timer.gen(self)
        if self.kconfig.get_symb('CLK') != 'n':
            clock.gen(self)
        if self.kconfig.get_symb('PINCTRL') != 'n':
            chip_pin.gen(self)
            self._cp2prj('pinmux.c')
        if self.kconfig.get_symb('RESETGEN') != 'n':
            rstgen.gen(self)
        if self.kconfig.get_symb('BOARD_REMAP') != 'n':
            remap.gen(self)

    def finish_project_gen(self):
        self._create_makefile(self.get_config_path('board.mk'),
                               os.path.join(self.prj_path, 'board.mk'))

    def _create_makefile(self, template, out_path):
        l = ['$(LOCAL_DIR)/' + f for f in os.listdir(self.prj_path) if f.endswith('.c')]
        srcs = self._get_string(l)
        template_fp = open(template, 'r', encoding='utf-8')
        template_data = template_fp.read()
        template_fp.close()
        ss = template_data.format(self.prj, self.board, self.arch, srcs)
        with MKFile(out_path) as out_mk:
            out_mk.write(ss)

    def _get_string(self, l):
        s = ''
        for v in l:
            s += ' \\\n\t' + v
        return s

    # copy file from data path to project path
    def _cp2prj(self, *argv):
        argn = len(argv)
        if argn == 1:
            # copy the file with the same name
            file_in_datapath = argv[0]
            file_in_projpath = argv[0]
        elif argn == 2:
            # copy the file and rename it to args[1]
            file_in_datapath = argv[0]
            file_in_projpath = argv[1]
        else:
            raise Exception("Number of args must be 1 or 2.")

        shutil.copyfile(self.get_config_path(file_in_datapath),
                        self.get_generated_path(file_in_projpath))
