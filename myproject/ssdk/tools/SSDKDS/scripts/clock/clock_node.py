#!/usr/bin/python
# -*- coding: UTF-8 -*-

class ClkCore(object):

    def __init__(self, name, base, type, ops):
        self.name = name
        self.data_name = "g_" + name + "_core"
        self.base = base
        self.type = type
        self.ops = ops

    def gen_struct(self):
        struct = """
static clk_core_t %s = {
    .base = %s,
    .ops = %s,
};
""" % (self.data_name, self.base, '&' + self.ops)
        return struct

    def gen_struct_declarine(self):
        declarine = """extern clk_core_t %s;""" % (self.data_name)
        return declarine

    def write_struct(self, src_file, inc_file):
        src_file.write(self.gen_struct())
        #inc_file.write(self.gen_struct_declarine())


class ClkNode(object):

    def __init__(self, name, core, id):
        self.name = name
        self.id = hex(id)
        self.core = core
        self.parent = None
        self.parents_list = None
        self.parents_num = 0
        self.mux = 0
        self.freq = 0
        self.prepare = False
        if core is not None and core.type != "pll":
            self.data_name = "g_ckgen_%s_%s" %(core.type, name)
        else:
            self.data_name = "g_" + name

    def set_freq(self, freq):
        self.freq = freq
        #if self.parent is not None:
            #if freq > self.parent.freq:
                #raise Exception("{}: freq > parent freq".format(self.name))

    def set_domain(self, domain):
        self.domain = domain

    def get_domain(self):
        return self.domain

    def set_prepare(self, prepare):
        self.prepare = prepare

    def get_prepare(self):
        return self.prepare

    def set_parents_list(self, parents_list):
        self.parents_list = parents_list
        self.parents_num = len(parents_list)

    def set_parent(self, mux):
        self.mux = mux
        name = self.parents_list[mux]
        parent_node = clk_node_list.get_node_by_name(name)
        if parent_node is None:
            raise Exception("{}: Cannot found input clk node {}".format(self.name, name))
        self.parent = parent_node

    def gen_struct(self):
        if self.core is not None:
            core_name = '&' + self.core.data_name
        else:
            core_name = "NULL"

        if self.parent is not None:
            parent_name = 'CLK_NODE(%s)' % self.parent.data_name
        else:
            parent_name = "NULL"

        struct = """
clk_node_t %s = {
    .core = %s,
    .id = %s,
    .rate = %s,
    .mux = %s,
    .parent = %s,
};""" % (self.data_name, core_name, self.id, self.freq, self.mux, parent_name)
        return struct

    def gen_struct_declarine(self):
        declarine = """
extern clk_node_t %s;
""" % (self.data_name)
        return declarine

    def write_struct(self, src_file, inc_file):
        src_file.write(self.gen_struct())
        inc_file.write(self.gen_struct_declarine())

class GatingNode(ClkNode):

    def __init__(self, name, core, id):
        super().__init__(name, core, id)
        self.data_name = "g_ckgen_%s_%s" %(self.core.type, self.name)
        self.enable = 0 # 0--not set, 1--enable, 2--disable

    def set_enable(self, enable):
        self.enable = enable

    def get_enable(self):
        return self.enable

    def gen_struct(self):
        core_name = '&' + self.core.data_name
        struct = """
clk_node_t %s = {
    .core = %s,
    .id = %s,
};""" % (self.data_name, core_name, self.id)
        return struct


class NodeList(object):

    def __init__(self):
        self.list = []

    def clear(self):
        self.list.clear()

    def add_node(self, node):
        for item in self.list:
            if item.data_name == node.data_name:
                return
        self.list.append(node)

    def insert_node(self, node, pre_node_name):
        for i, item in enumerate(self.list):
            if item.name == pre_node_name:
                self.list.insert(i, node)
                return
        self.list.append(node)

    def get_dataname_by_name(self, name):
        for node in self.list:
            if node.name == name:
                return node.data_name
        return None

    def get_node_by_name(self, name):
        for node in self.list:
            if node.name == name:
                return node
        return None

    def get_list_iter(self):
        return iter(self.list)

clk_core_list = NodeList()
clk_node_list = NodeList()
gating_node_list = NodeList()
