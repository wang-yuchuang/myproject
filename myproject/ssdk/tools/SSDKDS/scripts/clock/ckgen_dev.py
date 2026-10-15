#!/usr/bin/python
# -*- coding: UTF-8 -*-

from clock_node import ClkCore, ClkNode, GatingNode
from clock_node import clk_core_list, clk_node_list, gating_node_list

pll_id = {
    "vco"   :       0,
    "root"  :       1,
    "div_a" :       2,
    "div_b" :       3,
    "div_c" :       4,
    "div_d" :       5
}

pll_parent = {
    "vco"   :       "rc_24m",
    "root"  :       "vco",
    "div_a" :       "vco",
    "div_b" :       "vco",
    "div_c" :       "vco",
    "div_d" :       "vco"
}

kunlun_bus_div_list = ["root", "div_m", "div_n", "div_p", "div_q"]

kunlun_bus_id_mask = {
    'root'  :       0x0000000,
    'div_m' :       0x4000000,
    'div_n' :       0x3000000,
    'div_p' :       0x2000000,
    'div_q' :       0x1000000
}

kunlun_bus_div_mnpq = {
    'saf'   :       [1,2,4,1,1],
    'sec'   :       [1,2,8,1,1],
    'soc'   :       [1,2,6,6,8],
    'disp'  :       [1,1,2,1,1]
}

uuu_div_list = ["div_m", "div_n", "div_p", "div_q"]

uuu_id_mask = {
    'div_m':        0x3000000,
    'div_n':        0x2000000,
    'div_p':        0x1000000,
    'div_q':        0x0000000
}

uuu_prepare_list = {
    'cpu1a' :       'core',
    'cpu1b' :       'core',
    'cpu2'  :       'core',
    'gpu1'  :       'core',
    'gpu2'  :       'core',
    'vpu1'  :       'ip',
    'mjpeg' :       'ip',
    'vsn'   :       'bus',
    'vpu'   :       'bus'
}

kunlun_clk_ops = {
    'bus'     : 'ckgen_bus_ops',
    'ip'      : 'ckgen_ip_ops',
    'core'    : 'ckgen_core_ops',
    'uuu'     : 'ckgen_uuu_ops',
    'gating'  : 'ckgen_lp_gating_ops'
}


class ClkDev(object):

    def __init__(self, name, freq):
        self.name = name
        self.freq = freq
        self.prepare = False

    def gen_res(self):
        clk_node = clk_node_list.get_node_by_name(self.name)
        if clk_node is None:
            clk_node = ClkNode(self.name, None, 0)
            clk_node.set_freq(self.freq)
            clk_node.set_prepare(self.prepare)
            clk_node_list.add_node(clk_node)
            print(clk_node.gen_struct())

    def get_res_name(self):
        return self.name

    def set_prepare(self):
        self.prepare = True


class PllDev(ClkDev):

    def __init__(self, name, pin, domain, root_freq, freq):
        super().__init__(name, freq)
        self.pin = pin
        self.domain = domain
        self.root_freq = root_freq
        self.ops = 'sc_pfpll_ops'

    def __gen_pll_core_node__(self):
        core_name = self.name
        clk_core = clk_core_list.get_node_by_name(core_name)
        if clk_core is None:
            base = "APB_%s_BASE" % (self.name.upper())
            clk_core = ClkCore(self.name, base, "pll", self.ops)
            clk_core_list.add_node(clk_core)
            print(clk_core.gen_struct())
        return clk_core

    def __gen_pll_node__(self, pin_name, freq, clk_core):
        if pin_name is not None:
            clk_name = "%s_%s" % (self.name, pin_name)
            clk_node = ClkNode(clk_name, clk_core, pll_id[pin_name])
            parent_name = pll_parent[pin_name]
            if parent_name == "vco" or parent_name == "root":
                parent_name = "%s_%s" % (self.name, parent_name)
            clk_node.set_parents_list([parent_name])
            clk_node.set_parent(0)
        else:
            clk_node = ClkNode(self.name, None, 0)

        clk_node.set_domain(self.domain)
        clk_node.set_freq(freq)
        clk_node.set_prepare(self.prepare)
        clk_node_list.add_node(clk_node)
        print(clk_node.gen_struct())

    def gen_res(self):
        if "pll" in self.name:
            clk_core = self.__gen_pll_core_node__()

            if self.pin == "root":
                self.__gen_pll_node__("vco", self.root_freq, clk_core)

            self.__gen_pll_node__(self.pin, self.freq, clk_core)
        else:
            self.__gen_pll_node__(None, self.freq, None)

    def get_res_name(self):
        if "pll" in self.name:
            return "%s_%s" % (self.name, self.pin)
        else:
            return self.name


class CkgenDev(ClkDev):

    def __init__(self, name, domain, idx, freq, parent_list, select):
        super().__init__(name, freq)
        self.domain = domain
        self.idx = idx
        self.parent_list = parent_list
        self.select = select
        self.type = "ip"
        self.ops = kunlun_clk_ops

    def __gen_clk_core_node__(self):
        core_name = "%s_%s" % (self.domain, self.type)
        clk_core = clk_core_list.get_node_by_name(core_name)
        if clk_core is None:
            base = "APB_CKGEN_%s_BASE" % (self.domain.upper())
            clk_core = ClkCore(core_name, base, self.type, self.ops[self.type])
            clk_core_list.add_node(clk_core)
            print(clk_core.gen_struct())
        return clk_core

    def __get_ckgen_id__(self):
        return self.idx

    def set_ckgen_type(self, type):
        self.type = type

    def gen_res(self):

        #create clk_core
        clk_core = self.__gen_clk_core_node__()

        #create clk_node
        clk_node = ClkNode(self.name, clk_core, self.__get_ckgen_id__())

        clk_node.set_parents_list(self.parent_list)
        clk_node.set_parent(self.select)

        clk_node.set_domain(self.domain)
        clk_node.set_freq(self.freq)
        clk_node.set_prepare(self.prepare)

        if self.name in uuu_prepare_list and self.prepare == True:
            prepare_node = ClkNode(self.name + '_pre', clk_core, self.__get_ckgen_id__())
            prepare_node.set_parents_list(['rc_24m'])
            prepare_node.set_parent(0)
            prepare_node.set_domain(self.domain)
            prepare_node.set_freq(24000000)
            prepare_node.set_prepare(True)
            if self.name == 'vpu1' or self.name == 'mjpeg':
                pll_name = 'vpu'
            else:
                pll_name = self.name
            clk_node_list.insert_node(prepare_node, 'pll_%s_vco' % (pll_name))

        clk_node_list.add_node(clk_node)
        print(clk_node.gen_struct())


class BusDev(CkgenDev):

    def __init__(self, name, domain, idx, freq, parent_list, select):
        super().__init__(name, domain, idx, freq, parent_list, select)
        self.type = "bus"
        self.no_div = 0
        self.bus_id_mask = kunlun_bus_id_mask
        self.bus_div_mnpq = kunlun_bus_div_mnpq
        self.bus_div_list = kunlun_bus_div_list

    def __get_ckgen_id__(self, div_type):
        return self.bus_id_mask[div_type] + self.idx

    def __get_ckgen_bus_div__(self, div_type):
        div = 1
        if self.no_div == 0:
            div_mnpq = self.bus_div_mnpq[self.domain]
            div = div_mnpq[div_type]
        return div

    def set_no_div(self):
        self.no_div = 1

    def gen_res(self):
        #clk_core
        clk_core = self.__gen_clk_core_node__()

        #create root and divnmpq clk_node
        for i, div_type in enumerate(self.bus_div_list):
            clk_name = "%s_%s" % (self.name, div_type)
            clk_node = ClkNode(clk_name, clk_core, self.__get_ckgen_id__(div_type))
            clk_node.set_parents_list(self.parent_list)
            clk_node.set_parent(self.select)
            clk_node.set_domain(self.domain)
            clk_node.set_freq(int(self.freq / self.__get_ckgen_bus_div__(i)))
            clk_node.set_prepare(self.prepare)

            if self.name in uuu_prepare_list and self.prepare == True:
                prepare_node = ClkNode(clk_name + '_pre', clk_core, self.__get_ckgen_id__(div_type))
                prepare_node.set_parents_list(['rc_24m'])
                prepare_node.set_parent(0)
                prepare_node.set_domain(self.domain)
                prepare_node.set_freq(24000000)
                prepare_node.set_prepare(True)
                if self.name == 'vpu1' or self.name == 'mjpeg':
                    pll_name = 'vpu'
                else:
                    pll_name = self.name
                clk_node_list.insert_node(prepare_node, 'pll_%s_vco' % (pll_name))

            clk_node_list.add_node(clk_node)
            print(clk_node.gen_struct())


class UUUDev(CkgenDev):

    def __init__(self, name, freq, idx, cko_idx):
        if name == "vpu1" or name == "mjpeg":
            parent_list = ["vpu", "pll_vpu_root"]
        else:
            parent_list = [name, "pll_%s_root" % (name)]
        super().__init__(name, "soc", idx, freq,
                         parent_list, 1)
        self.div_type = uuu_div_list[cko_idx]
        self.type = "uuu"

    def __get_ckgen_id__(self):
        return uuu_id_mask[self.div_type] + self.idx

    def gen_res(self):
        #clk_core
        clk_core = self.__gen_clk_core_node__()

        #create clk_node
        clk_name = "%s_%s" % (self.name, self.div_type)
        clk_node = ClkNode(clk_name, clk_core, self.__get_ckgen_id__())

        clk_node.set_parents_list(self.parent_list)
        clk_node.set_parent(self.select)

        clk_node.set_domain(self.domain)
        clk_node.set_freq(self.freq)
        clk_node.set_prepare(self.prepare)

        if self.name in uuu_prepare_list and self.prepare == True:
            prepare_node = ClkNode(clk_name + '_pre', clk_core, self.__get_ckgen_id__())
            if uuu_prepare_list[self.name] == 'bus':
                prepare_node.set_parents_list([self.name + '_root_pre'])
            else:
                prepare_node.set_parents_list([self.name + '_pre'])
            prepare_node.set_parent(0)
            prepare_node.set_domain(self.domain)
            prepare_node.set_freq(24000000)
            prepare_node.set_prepare(True)
            if self.name == 'vpu1' or self.name == 'mjpeg':
                pll_name = 'vpu'
            else:
                pll_name = self.name
            clk_node_list.insert_node(prepare_node, 'pll_%s_vco' % (pll_name))

        clk_node_list.add_node(clk_node)
        print(clk_node.gen_struct())


class GatingDev(CkgenDev):

    def __init__(self, name, idx, domain, enable):
        super().__init__(name, domain, idx, 0, None, 0)
        self.enable = enable
        self.type = "gating"

    def gen_res(self):
        #clk_core
        clk_core = self.__gen_clk_core_node__()

        #create gating_node
        gating_node = GatingNode(self.name, clk_core, self.idx)
        gating_node.set_enable(self.enable)

        gating_node_list.add_node(gating_node)
        print(gating_node.gen_struct())

