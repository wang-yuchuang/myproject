# -*- coding: utf-8 -*-

import xml.dom.minidom

class ChipPinInfo(object):

    def __init__(self, xml_file):
        self.file_path = xml_file
        self.mux_info = {}
        self.input_sel = []

    def load_chip_pin_info(self):
        dom_tree = xml.dom.minidom.parse(self.file_path)
        root_element = dom_tree.documentElement
        pins = root_element.getElementsByTagName("pin")

        for pin in pins:
            index = pin.getAttribute("index")
            name = pin.getAttribute("name")
            ball = pin.getAttribute("ball")

            self.mux_info[name] = [index, {}, ball]

            muxs = pin.getElementsByTagName("mux")

            for mux in muxs:
                id = mux.getAttribute("id")
                func = mux.getAttribute("func")
                offset = mux.getAttribute("offset")
                val = mux.getAttribute("val")

                self.mux_info[name][1][func] = id

                if offset != "":
                    self.input_sel.append([index, id, offset, val])
