# -*- coding: utf-8 -*-

import os
import xlrd

class ExcelBase(object):
    col_info = {}

    def __init__(self, xls_book, sheet_name):
        self.book = xls_book
        self.sheet_name = sheet_name
        self.sheet = None
        self.res_list = []

    def init_sheet_info(self):
        self.sheet = self.book.sheet_by_name(self.sheet_name)
        if self.sheet is None:
            raise Exception("[Error] Failed to open sheet : %s" % self.sheet_name)
        self._get_column_index_info()
        is_valid = self._is_valid_res_sheet()
        return is_valid

    def _get_column_index_info(self):
        for key in self.col_info:
            for i in range(0, self.sheet.ncols):
                col_value = self.sheet.cell(0, i).value
                if col_value == key:
                    self.col_info[key] = i
                    break

    def _is_valid_res_sheet(self):
        """
        for key in self.col_info:
            if self.col_info[key] == -1:
                raise Exception("[Error] Not find \"{}\" column in \"{}\"".format(key, self.sheet_name))
        """
        return True

    def load_res_list(self, start_row):
        for i in range(start_row, self.sheet.nrows):
            item = {}
            for key in self.col_info:
                if self.col_info[key] != -1:
                    col_value = self.sheet.cell(i, self.col_info[key]).value
                    item[key] = col_value
            self.res_list.append(item)
        return self.res_list
