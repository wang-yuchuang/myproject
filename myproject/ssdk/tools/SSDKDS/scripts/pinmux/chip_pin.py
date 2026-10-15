# -*- coding: utf-8 -*-

import os
import sys
import xlrd
import xml.dom.minidom
sys.path.append(os.path.dirname(__file__))
from chip_pin_info import ChipPinInfo
from file_ops import FileBase
from new_excel import NewExcel
from const_pin import const_pin
from pin_config_excel import PinConfigExcel
import project

class PinIncFile(FileBase):
    prefix = FileBase.STATEMENT + const_pin.head_prefix
    suffix = const_pin.head_suffix

class PinSrcFile(FileBase):
    prefix = FileBase.STATEMENT + const_pin.source_prefix
    suffix = const_pin.source_suffix

class ChipPinGen(object):

    def __init__(self, config_file):
        self.file = config_file
        self.info = None
        self.dom = None
        self.num = 0
        self.code = ""
        self.src_sel_num = 0
        self.src_sel_code = ""

    def load_config_data(self, chip_info_file):
        self.info = ChipPinInfo(chip_info_file)
        self.info.load_chip_pin_info()
        self.dom = xml.dom.minidom.parse(self.file)

    def generate_config_code(self):
        root_element = self.dom.documentElement
        pins = root_element.getElementsByTagName("pin")

        self.num = len(pins)
        self.src_sel_num = len(self.info.input_sel)

        for pin in pins:
            pin_name = self.get_element_text(pin, "name")
            mux_func = self.get_element_text(pin, "mux_function")

            self.code += "    /* {}, Pin {}, {}, Mux:{} */\n" .format(
                self.info.mux_info[pin_name][2], self.info.mux_info[pin_name][0],
                pin_name, mux_func
            )

            self.code += "    {\n"

            self.code += "        .pin_index         =  %sU,\n" % (
                self.info.mux_info[pin_name][0]
            )

            self.insert_config_item(pin, "gpio_controller")

            self.code += "        .mux               =  %s,\n" % (
                const_pin.mux[self.info.mux_info[pin_name][1][mux_func]]
            )

            self.insert_config_item(pin, "open_drain")
            self.insert_config_item(pin, "pull_select")
            self.insert_config_item(pin, "drive_strength")
            self.insert_config_item(pin, "slew_rate")
            self.insert_config_item(pin, "input_select")
            self.insert_config_item(pin, "direction")
            self.insert_config_item(pin, "interrupt")
            self.insert_config_item(pin, "initial_value")
            self.insert_config_item(pin, "force_input")

            self.code += "    },\n\n"

        self.code = self.code[:-1]

        for src_sel in self.info.input_sel:
            self.src_sel_code += "    {"
            self.src_sel_code += "{:>4s},{:>2s},{:>6s},{:>2s}" .format(
                src_sel[0], src_sel[1], src_sel[2], src_sel[3]
            )
            self.src_sel_code += "  },\n"


    def generate_taishan_config_code(self):
        cons = {'PIN_OPEN_DRAIN_DISABLED':'PIN_PUSH_PULL',
                'PIN_OPEN_DRAIN_ENABLED':'PIN_OPEN_DRAIN',
                'PIN_PULL_NOT_ENABLED':'PIN_NOPULL',
                'PIN_PULL_DOWN_ENABLED':'PIN_PULL_DOWN',
                'PIN_PULL_UP_ENABLED':'PIN_PULL_UP',
                'PIN_DRIVE_STRENTH_2MA':'PIN_DS_2MA',
                'PIN_DRIVE_STRENTH_8MA':'PIN_DS_8MA',
                'PIN_DRIVE_STRENTH_4MA':'PIN_DS_4MA',
                'PIN_DRIVE_STRENTH_12MA':'PIN_DS_12MA',
                'PIN_FAST_SLEW_RATE':'PIN_SR_FAST',
                'PIN_SLOW_SLEW_RATE':'PIN_SR_SLOW',
                'PIN_INPUT_CMOS_SCHMITT_MODE':'PIN_IS_CMOS_SCHMITT',
                'PIN_INPUT_CMOS_MODE':'PIN_IS_CMOS'}
        root_element = self.dom.documentElement
        pins = root_element.getElementsByTagName("pin")
        self.num = len(pins)
        for pin in pins:
            pin_name = self.get_element_text(pin, "name")
            mux_func = self.get_element_text(pin, "mux_function")
            self.code += "    /* Pin {}, {}, Mux:{} */\n" .format(
                self.info.mux_info[pin_name][0], pin_name, mux_func)
            self.code += "    {\n"
            self.code += "        .pin_index         =  %s,\n" % (pin_name)
            self.code += "        .mux               =  %s,\n" % (
                const_pin.mux[self.info.mux_info[pin_name][1][mux_func]])
            self.insert_config_item(pin, "open_drain", cons)
            self.insert_config_item(pin, "pull_select", cons)
            self.insert_config_item(pin, "drive_strength", cons)
            self.insert_config_item(pin, "slew_rate", cons)
            self.insert_config_item(pin, "input_select", cons)
            self.insert_config_item(pin, "direction")
            self.insert_config_item(pin, "interrupt")
            self.insert_config_item(pin, "initial_value")
            self.insert_config_item(pin, "force_input")
            self.code += "    },\n\n"
        self.code = self.code[:-1]

    def get_element_text(self, root, tag):
        text = ""

        elements = root.getElementsByTagName(tag)
        if len(elements) != 0:
            text += elements[0].childNodes[0].data

        return text

    def insert_config_item(self, root, tag, convert = None):
        value = self.get_element_text(root, tag)
        if value != "":
            s = const_pin.relation[tag][1][value]
            if convert != None:
                s = convert[s]
            self.code += "        .%s=  %s,\n" % (
                const_pin.relation[tag][0], s)

    def generate_head_file(self, file_name):
        write_data = "#define NUM_OF_CONFIGURED_PINS  %d\n" % (self.num)
        write_data += "#define NUM_OF_SOURCE_SELECT    %d\n" % (self.src_sel_num)
        with PinIncFile(file_name) as file_handler:
            file_handler.write(write_data)

    def generate_source_file(self, file_name):
        with PinSrcFile(file_name) as file_handler:
            file_handler.write(self.code)
            file_handler.write(const_pin.source_mid)
            file_handler.write(self.src_sel_code)


    def generate_taishan_head_file(self, file_name):
        # write_data = "#define NUM_OF_CONFIGURED_PINS  %d\n" % (self.num)
        write_data = """#ifdef CONFIG_WORK_ON_Z1
#define NUM_OF_CONFIGURED_PINS  214
#else
#define NUM_OF_CONFIGURED_PINS  {}
#endif
""" .format(self.num)
        PinIncFile.prefix = FileBase.STATEMENT + const_pin.taishan_head_prefix
        PinIncFile.suffix = const_pin.taishan_head_suffix
        with PinIncFile(file_name) as file_handler:
            file_handler.write(write_data)

    def generate_taishan_source_file(self, file_name):
        PinSrcFile.prefix = FileBase.STATEMENT + const_pin.taishan_source_prefix
        with PinSrcFile(file_name) as file_handler:
            file_handler.write(self.code)

    def xlsx_add_dropdown_list(self, pin, xlsx):
        name = self.get_element_text(pin, "name")
        index = self.info.mux_info[name][0]
        muxs = self.info.mux_info[name][1]
        ball = self.info.mux_info[name][2]
        xlsx.add_dropdown_list(const_pin.excel_mux, list(muxs.keys()))
        xlsx.fill_cell(const_pin.excel_ball, ball)
        for key, value in const_pin.xml_excel.items():
            if value[1] == {}:
                continue
            choice = list(value[1].values())
            if key in const_pin.xml_excel_emmc:
                if int(index) >= const_pin.emmc_pad_start:
                    choice = const_pin.xml_excel_emmc[key]
                else:
                    tmp = const_pin.xml_excel_emmc[key]
                    choice = [v for v in choice if v not in tmp]
            choice.sort()
            xlsx.add_dropdown_list(value[0], choice)

    def xlsx_write_row(self, pin, xlsx):
        for key, value in const_pin.xml_excel.items():
            text = self.get_element_text(pin, key)
            if value[1] != {}:
                text = value[1][text]
            xlsx.fill_cell(value[0], text)

    def save_as_xlsx(self, file_name):
        xlsx = NewExcel(file_name)
        xlsx.sheet_init(const_pin.excel_first_row, const_pin.default_sheet_name)
        root_element = self.dom.documentElement
        pins = root_element.getElementsByTagName("pin")
        for pin in pins:
            self.xlsx_add_dropdown_list(pin, xlsx)
            self.xlsx_write_row(pin, xlsx)
            xlsx.next_row()
        xlsx.close()


def gen(project):
    para1 = project.get_config_path('pinmux.dat')
    para2 = project.get_config_path('pinmux.xml')
    para3 = project.get_generated_path('pinmux_cfg.h')
    para4 = project.get_generated_path('pinmux_cfg.c')
    if project.platform == 'kunlun':
        main([para1, para2, para3, para4])
    elif project.platform == 'taishan':
        taishan_main([para1, para2, para3, para4])


def main(argv):
    exit_code = 0

    tmp_file = "tmp.xml"
    book = xlrd.open_workbook(argv[0])
    excel = PinConfigExcel(book, const_pin.default_sheet_name)
    excel.load_res_data()
    excel.save_as_xml(tmp_file)
    gen = ChipPinGen(tmp_file)
    gen.load_config_data(argv[1])
    gen.generate_config_code()
    gen.generate_head_file(argv[2])
    gen.generate_source_file(argv[3])
    os.remove(tmp_file)

    return exit_code


def taishan_main(argv):
    tmp_file = "tmp.xml"
    book = xlrd.open_workbook(argv[0])
    excel = PinConfigExcel(book, const_pin.default_sheet_name)
    excel.load_res_data()
    excel.save_as_xml(tmp_file)
    gen = ChipPinGen(tmp_file)
    gen.load_config_data(argv[1])
    gen.generate_taishan_config_code()
    gen.generate_taishan_head_file(argv[2])
    gen.generate_taishan_source_file(argv[3])
    os.remove(tmp_file)

if __name__ == '__main__':
    main(sys.argv[1:])
