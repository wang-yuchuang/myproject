# -*- coding: utf-8 -*-

import xlsxwriter

class NewExcel(object):

    def __init__(self, file_name):
        self.file = file_name
        self.book = None
        self.sheet_name = ""
        self.sheet = None
        self.col_info = {}
        self.row_cnt = 0

    def close(self):
        if self.book:
            self.book.close()

    def get_rows(self):
        return self.row_cnt

    def next_row(self):
        self.row_cnt += 1
        return self.row_cnt

    def sheet_init(self, first_row, sheet_name = ""):
        self.book = xlsxwriter.Workbook(self.file)
        if sheet_name == "":
            self.sheet = self.book.add_worksheet()
        else:
            self.sheet = self.book.add_worksheet(sheet_name)
            self.sheet_name = sheet_name
        for index, value in enumerate(first_row):
            self.col_info[value] = index
        self.sheet.set_column(0, len(first_row)-1, 20)
        bold = self.book.add_format({'bold': True})
        self.sheet.write_row(0, 0, first_row, bold)
        self.row_cnt += 1

    def add_dropdown_list(self, col_name, choice_list):
        self.sheet.data_validation(self.row_cnt, self.col_info[col_name],
        self.row_cnt, self.col_info[col_name], {'validate':'list','source':choice_list})

    def fill_cell(self, col_name, text_str):
        self.sheet.write(self.row_cnt, self.col_info[col_name], text_str)

    def write_row(self, row_tuple):
        self.sheet.write_row(self.row_cnt, 0, row_tuple)
        self.row_cnt += 1
