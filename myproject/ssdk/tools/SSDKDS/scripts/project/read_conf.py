# -*- coding: utf-8 -*-

class ReadConf(object):

    def __init__(self, file):
        self.file = file
        self.conf = {}

    def get_conf(self):
        with open(self.file, 'r', encoding='utf-8') as f:
            for line in f:
                arr = line.rstrip().split('=')
                if arr[0] not in self.conf:
                    self.conf[arr[0]] = arr[1]
                else:
                    raise Exception('config duplicate')
        return self.conf

    def update_conf(self, file):
        with open(file, 'r', encoding='utf-8') as f:
            for line in f:
                arr = line.rstrip().split('=')
                self.conf[arr[0]] = arr[1]
        return self.conf
