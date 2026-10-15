#!/usr/bin/python3
# -*- coding: UTF-8 -*-

import xlrd
import xlsxwriter

header_format = {
            'font_name': 'Times New Roman',
            'border': 1,
            'fg_color': '#74AC4C',
            'bold': True,
            'text_wrap': True,
            'align': 'center',
            'valign': 'vcenter',
            }
cell_format = {
            'font_name': 'Times New Roman',
            'border': 1,
            'text_wrap': True,
            'align': 'center',
            'valign': 'vcenter',
            }

class WorkBookWrite(object):

    def __init__(self, path):
        self.workbook = xlsxwriter.Workbook(path)

    def close(self):
        self.workbook.close()


class WorkBookRead(object):

    def __init__(self, path):
        self.workbook = xlrd.open_workbook(path)

    def check_sheet_exist(self, sheet_name):
        return (sheet_name in self.workbook.sheet_names())


class SheetWrite(object):

    def __init__(self, workbook, sheet_name):
        self.workbook = workbook.workbook
        self.sheet = self.workbook.add_worksheet(sheet_name)
        self.header_format = self.workbook.add_format(header_format)
        self.cell_format = self.workbook.add_format(cell_format)
        self.cur_row = 0

    def add_row(self):
        self.cur_row += 1

    def write_sheet_header(self, header_items):
        hander_len = len(header_items)
        self.sheet.set_column(0, hander_len - 1, 25)
        self.sheet.set_row(0, 30)
        for name, header in header_items.__members__.items():
            self.sheet.write(0, header.value, name, self.header_format)
        self.add_row()

    def write_sheet_cell_data(self, col, data):
        self.sheet.write(self.cur_row, col, data, self.cell_format)

    def write_sheet_cell_list(self, col, list, data):
        self.sheet.data_validation(self.cur_row, col, self.cur_row, col,
                {'validate': 'list', 'source': list})
        self.sheet.write(self.cur_row, col, data, self.cell_format)


class SheetRead(object):
    col_info = {}

    def __init__(self, workbook, sheet_name):
        self.workbook = workbook.workbook
        self.sheet = self.workbook.sheet_by_name(sheet_name)
        self.cur_row = 1
        self.res_list = []

    def read_row(self, row):
        return self.sheet.row_values(row, start_colx=0, end_colx=None)

    def read_cell(self, row, col):
        return self.sheet.cell_value(row, col)

    def read_cell_by_key(self, row, key):
        col = self.col_info[key]
        return self.sheet.cell_value(row, col)

    def add_row(self):
        self.cur_row += 1

    def get_rows(self):
        return self.sheet.nrows

    def init_col_index_info(self, row_idx):
        for key in self.col_info:
            for i in range(0, self.sheet.ncols):
                if key in self.sheet.cell(row_idx, i).value:
                    self.col_info[key] = i
                    break

    def load_res_list(self, start_row):
        for i in range(start_row, self.sheet.nrows):
            item = {}
            for key in self.col_info:
                if self.col_info[key] != -1:
                    item[key] = self.sheet.cell(i, self.col_info[key]).value
                else:
                    item[key] = None
            self.res_list.append(item)


# =============================================================================
#
# Openpyxl Excel Editor
#
# =============================================================================

import io
import os
from openpyxl import Workbook, load_workbook
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.worksheet.dimensions import ColumnDimension, RowDimension, DimensionHolder
from openpyxl.utils import get_column_letter
from openpyxl.styles import Font


class PyxlExcelReader:
    """
    Excel reader based on Openpyxl.

    FOR OLD PYTHON USERS:
    Please upgrade your Python version to 3.6 or later for newest Openpyxl
    support. Openpyxl 2.6.4 is the final release for Python 2.7 and 3.5.

    NOTICE FOR USING:
    Please use 'with' statement for instantiating this class.
    Example:
        with PyxlExcelReader(file) as reader:
            dict = read_sheet_into_dict('some sheet')
        # obj 'reader' dies once being out of the 'with' statement
        # No need for calling 'del reader'!
    """

    def __init__(self, path):
        self._excel_handle = open(path, 'rb')
        self._workbook = load_workbook(
            io.BytesIO(self._excel_handle.read()), read_only=True)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self._excel_handle.close()

    def read_sheet_into_dict(self, sheet_name):
        sheet = self._workbook[sheet_name]

        # a typical data_dict is like:
        # {'name': (name_of_1st_uart_config, name_of_2nd_uart_config, ...),
        #  'baudrate': (buad_of_1st_uart_config, ...),
        #  ...}
        data_dict = {col[0]: col[1:] for col in zip(*sheet.values)}
        return data_dict

    def get_sheetnames(self):
        return self._workbook.sheetnames


class PyxlExcelWriter:
    """
    Excel writer based on Openpyxl.

    FOR OLD PYTHON USERS:
    Please upgrade your Python version to 3.6 or later for newest Openpyxl
    support. Openpyxl 2.6.4 is the final release for Python 2.7 and 3.5.

    NOTICE FOR USING:
    Please use 'with' statement for instantiating this class.
    Example:
        with PyxlExcelWriter(file) as excel:
            create_template_sheet(...)
        # obj 'excel' dies once being out of the 'with' statement
        # No need for calling 'del excel'!
    """



    def __init__(self, filename):
        self.filename = filename

        if os.path.isfile(filename):
            self._workbook = load_workbook(filename)
        else:
            self._workbook = Workbook()
            # remove the default sheet 'Sheet'
            del self._workbook['Sheet']

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self._workbook.save(self.filename)

    def append_rows(self, sheet_name, rows_list):
        sheet = self._workbook[sheet_name]
        # rows_list must be a list of lists
        is_legal = all(isinstance(elem, list) for elem in rows_list)
        if not is_legal:
            raise TypeError('rows_list·must·be·a·list·of·lists')
        for row in rows_list:
            sheet.append(row)

    def set_auto_width(self, sheet_name):
        sheet = self._workbook[sheet_name]
        dh_column = DimensionHolder(worksheet=sheet)
        for column_cells in sheet.columns:
            idx = column_cells[0].col_idx
            length = max(len(str(cell.value)) for cell in column_cells)
            # the width of a column depends on the longest cell in it
            dh_column[get_column_letter(idx)] = \
                ColumnDimension(sheet, min=idx, max=idx, width=length*1.23)
        sheet.column_dimensions = dh_column

    def create_template_sheet(self, sheet_name, template, default_cfg):
        # if the sheet already exists, delete it first
        if sheet_name in self._workbook.sheetnames:
            del self._workbook[sheet_name]
        self._workbook.create_sheet(sheet_name)
        sheet = self._workbook[sheet_name]

        # create a bold header
        header = [item[0] for item in template]
        sheet.append(header)
        for cell in sheet[1]:
            cell.font = Font(bold=True)

        # set up the available choices for each item
        for idx, item in enumerate(template):
            choice = item[1]
            if choice is None:
                dv = DataValidation(allow_blank=True)
            else:
                dv = DataValidation(
                    type='list',
                    formula1=choice,
                    allow_blank=True
                )
            sheet.add_data_validation(dv)
            # get_column_letter(1) == 'A'
            col_letter = get_column_letter(idx + 1)
            dv.add('%s2:%s1048576' % (col_letter, col_letter))

        # append default configs
        self.append_rows(sheet_name, default_cfg)
        self.set_auto_width(sheet_name)
