import os
import sys

sys.path.append(os.path.dirname(__file__))
import kconfiglib

from Kconfiglib import menuconfig

class SDKconfig(object):

    def __init__(self, defconfig_path, kconfig_path, out_path):
        self.kconfig_path = os.path.join(kconfig_path, 'Kconfig')
        self.defconfig_path = os.path.join(defconfig_path, 'defconfig')
        self.config_h_path = os.path.join(out_path, 'config.h')
        self.kconf = kconfiglib.Kconfig(self.kconfig_path,
                                        suppress_traceback=True)
        os.environ["KCONFIG_CONFIG"] = os.path.join(out_path, '.config')
        if os.path.isfile(self.defconfig_path):
            self.kconf.load_config(self.defconfig_path)
            self.kconf.write_config()
        else:
            raise Exception('Unknown defconfig!')

    def gen_config_h(self):
        self.kconf.write_autoconf(self.config_h_path)

    def menuconfig(self):
        menuconfig.menuconfig(self.kconf)
        self.kconf.load_config()
        self.kconf.write_min_config(self.defconfig_path)
        self.kconf.write_autoconf(self.config_h_path)

    def guiconfig(self):
        from Kconfiglib import guiconfig
        guiconfig.menuconfig(self.kconf)
        self.kconf.load_config()
        self.kconf.write_min_config(self.defconfig_path)
        self.kconf.write_autoconf(self.config_h_path)

    def get_symb(self, symb):
        for item in self.kconf.unique_defined_syms:
            if item.name == symb:
                return item.str_value
        return 'n'
