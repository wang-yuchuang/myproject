# -*- coding: utf-8 -*-

import sys
import xlrd
import xml.dom.minidom
from read_excel import ExcelBase
from const_pin import const_pin

class PinConfigExcel(ExcelBase):
    col_info = {
        const_pin.excel_pin : -1,
        const_pin.excel_ball : -1,
        const_pin.excel_mux : -1,
        const_pin.excel_controller : -1,
        const_pin.excel_ode : -1,
        const_pin.excel_pull : -1,
        const_pin.excel_ds : -1,
        const_pin.excel_sr : -1,
        const_pin.excel_is : -1,
        const_pin.excel_fi : -1,
        const_pin.excel_dir : -1,
        const_pin.excel_int : -1,
        const_pin.excel_init : -1
    }

    def __init__(self, xls_book, sheet_name):
        super().__init__(xls_book, sheet_name)

    def load_res_data(self):
        self.init_sheet_info()
        self.load_res_list(1)

    def save_as_xml(self, file_name):
        doc = xml.dom.minidom.Document()
        pins = doc.createElement("pins")
        doc.appendChild(pins)
        for row in self.res_list:
            if row[const_pin.excel_mux] == '':
                continue
            pin = doc.createElement("pin")
            pins.appendChild(pin)
            for key in const_pin.excel_first_row:
                if key == const_pin.excel_ball:
                    continue
                if key not in row:
                    continue
                text = row[key]
                value = const_pin.excel_xml[key]
                if value[1] != {}:
                    reverse = dict([(v,k) for k,v in value[1].items()])
                    text = reverse[text]
                node = doc.createElement(value[0])
                pin.appendChild(node)
                text_node = doc.createTextNode(text)
                node.appendChild(text_node)
        fp = open(file_name, 'w+', newline='\n', encoding='utf-8')
        doc.writexml(fp, indent='', addindent='\t', newl='\n', encoding='utf-8')
        fp.close()


def main(argv):
    book = xlrd.open_workbook(argv[0])
    excel = PinConfigExcel(book, const_pin.default_sheet_name)
    excel.load_res_data()
    excel.save_as_xml(argv[1])

if __name__ == '__main__':
    main(sys.argv[1:])
