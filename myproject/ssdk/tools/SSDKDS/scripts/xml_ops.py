#!/usr/bin/python
# -*- coding: UTF-8 -*-

try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET

class XmlBase(object):

    def __init__(self, path):
        self.path = path
        self.tree = ET.ElementTree(file=path)
        self.root = self.tree.getroot()

    def xml_write(self):
        self.tree.write(self.path, encoding="utf-8", xml_declaration=True)

    def get_root(self):
        return self.root

    def get_attrib(self, node, key):
        return node.get(key, default=None)

    def set_attrib(self, node, key, value):
        node.set(key, value)

    def get_text(self, node, name):
        return node.findtext(name)

    def set_text(self, node, name, text):
        sub_node = node.find(name)
        if sub_node is not None:
            sub_node.text = text

    def find_nodes(self, node, path):
        return node.findall(path)
