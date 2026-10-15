# -*- coding: utf-8 -*-

import os
from const_mk import const_mk

class Config(object):

    root = ''
    cfg = {}
    seq = []

    def __init__(self):
        self.child = []

    def add_path(self, path):
        Config.root = path

    def add_item(self, var, val):
        var = var.strip()
        val = val.strip()
        Config.cfg[var] = val
        Config.seq.append(var)

    def gen_head(self):
        s = ''
        for k in Config.seq:
            if k in const_mk.string_list:
                continue
            v = Config.cfg[k]
            if v == 'y':
                v = '1'
            if v != 'n':
                s += '#define {} {}\n' .format(k, v)
        return s

def main():
    cfg = Config()
    cfg.add_path('.')
    cfg.add_item('ARCH', 'arm')
    l, d = get_default()
    print(cfg.gen_mk())
    print(cfg.gen_head())

if __name__ == '__main__':
    main()
