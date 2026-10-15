#!/usr/bin/python
# -*- coding: UTF-8 -*-

from excel_ops import SheetRead
from ckgen_dev import ClkDev, PllDev, CkgenDev, BusDev, UUUDev, GatingDev
from taishan_ckgen_dev import TaishanPllDev, TaishanCkgenDev, TaishanBusDev, TaishanGatingDev
from clock_node import clk_node_list

def convert_root_name(root_name):
    root_name = root_name.replace("(", "")
    root_name = root_name.replace(" ", "_")
    root_name = root_name.replace(",", "")
    root_name = root_name.replace(")", "")
    return root_name

def convert_freq(freq):
    return int(round(freq, 4) * 1000000)

class PllSheet(SheetRead):

    col_info = {
        "Src inst":     -1,
        "Source Pin":   -1,
        "Des Inst":     -1,
        "Des Pin":      -1,
        "PLL CFG":      -1,
        "PLL DIV CFG":  -1,
        "CLK FREQ":     -1,
        "available":    -1,
        "prepare":      -1
    }

    def __init__(self, workbook, sheet_name, domain):
        super().__init__(workbook, sheet_name)
        self.domain = domain.lower()
        self.ckin_map = {}

    def preprocessing(self, start_row):
        super().init_col_index_info(start_row)
        self.load_res_list(start_row + 1)

    def gen_res(self, start_row, platform):

        self.preprocessing(start_row)

        pre_pll_name = ''
        for item in self.res_list:
            pll_name = item["Src inst"].lower()
            if pll_name == '':
                continue

            if "available" in item and item["available"] == "N":
                continue

            root_freq = convert_freq(item["PLL CFG"])

            if item["CLK FREQ"] is not None:
                clk_freq = convert_freq(item["CLK FREQ"])
            else:
                clk_freq = root_freq

            if platform == 'kunlun':
                pll_dev = PllDev(pll_name, item["Source Pin"],
                                 self.domain, root_freq, clk_freq)
            elif platform == 'taishan':
                pll_dev = TaishanPllDev(pll_name, item["Source Pin"],
                                        self.domain, root_freq, clk_freq)
            else:
                raise Exception("pll platform error: {}".format(platform))

            if "prepare" in item and item["prepare"] == "Y":
                pll_dev.set_prepare()

            pll_dev.gen_res()
            self.ckin_map[item["Des Pin"]] = pll_dev.get_res_name()


class CkgenSheet(SheetRead):

    col_info = {
        "slc_idx":      -1,
        "slc_type":     -1,
        "ck_in":        -1,
        "ROOT NAME":    -1,
        "UUU_PRESENT":  -1,
        "Selector":     -1,
        "SelClk":       -1,
        "available":    -1,
        "prepare":      -1
    }

    ckgen_num = {
        "bus":          0,
        "ip":           0,
        "core":         0,
        "uuu":          0
    }

    def __init__(self, workbook, sheet_name, pll_sheet, domain):
        super().__init__(workbook, sheet_name)
        self.pll_sheet = pll_sheet
        self.domain = domain.lower()
        self.ckin_scol = 0
        self.ckin_ecol = 0
        self.ckin_lists = []
        self.ckgen_idx_list = {}
        self.ckgen_cnt = {
            "bus":          0,
            "ip":           0,
            "core":         0
        }

    def preprocessing(self, start_row):
        super().init_col_index_info(start_row)
        self.load_ckin_list(start_row)
        self.load_res_list(start_row + 1)

    def load_ckin_list(self, start_row):
        self.ckin_scol = self.col_info["ck_in"]
        self.ckin_ecol = self.col_info["ROOT NAME"] - 1
        for row in range(start_row + 1, self.get_rows()):
            item = [None]*(self.ckin_ecol - self.ckin_scol)
            for col in range(self.ckin_scol, self.ckin_ecol):
                ckin_idx = self.read_cell(row, col)
                if isinstance(ckin_idx, float):
                    item[int(ckin_idx)] = self.read_cell(start_row, col)
            item = [i for i in item if i != None]
            self.ckin_lists.append(item)

    def convert_ckin_list(self, ckin_list):
        parent_list = []
        for ckin in ckin_list:
            parent_list.append(self.pll_sheet.ckin_map[ckin])
        return parent_list

    def set_ckgen_idx(self, type, name):
        self.ckgen_idx_list[name] = self.ckgen_cnt[type]
        self.ckgen_cnt[type] += 1

    def get_ckgen_idx(self, name):
        return self.ckgen_idx_list[name]

    def gen_res(self, start_row, platform):

        self.preprocessing(start_row)

        for i, item in enumerate(self.res_list):
            ckgen_type = item["slc_type"].lower()
            name = convert_root_name(item["ROOT NAME"].lower())
            self.set_ckgen_idx(ckgen_type, name)

            if "available" in item and item["available"] == "N":
                continue

            parent_list = self.convert_ckin_list(self.ckin_lists[i])
            selector = int(item["Selector"])
            if item["UUU_PRESENT"] is not None and int(item["UUU_PRESENT"]) == 1 and selector > 3:
                selector = 3
                parent_node = clk_node_list.get_node_by_name(parent_list[selector])
                freq = parent_node.freq
                no_div = 1
            else:
                freq = convert_freq(item["SelClk"])
                no_div = 0

            idx = self.get_ckgen_idx(name)

            if ckgen_type == "ip":
                if platform == 'kunlun':
                    ckgen_dev = CkgenDev(name, self.domain, idx,
                                         freq, parent_list, selector)
                elif platform == 'taishan':
                    ckgen_dev = TaishanCkgenDev(name, self.domain, idx,
                                                freq, parent_list, selector)
                else:
                    raise Exception("ip ckgen platform error: {}".format(platform))
            elif ckgen_type == "bus":
                if platform == 'kunlun':
                    ckgen_dev = BusDev(name, self.domain, idx,
                                       freq, parent_list, selector)
                elif platform == 'taishan':
                    ckgen_dev = TaishanBusDev(name, self.domain, idx,
                                              freq, parent_list, selector)
                else:
                    raise Exception("bus ckgen platform error: {}".format(platform))

                if no_div == 1:
                    ckgen_dev.set_no_div()
            elif ckgen_type == "core":
                if platform == 'kunlun':
                    ckgen_dev = CkgenDev(name, self.domain, idx,
                                         freq, parent_list, selector)
                elif platform == 'taishan':
                    ckgen_dev = TaishanCkgenDev(name, self.domain, idx,
                                                freq, parent_list, selector)
                else:
                    raise Exception("core ckgen platform error: {}".format(platform))
                ckgen_dev.set_ckgen_type(ckgen_type)
            else:
                raise Exception("ckgen type error: {}".format(ckgen_type))

            if "prepare" in item and item["prepare"] == "Y":
                ckgen_dev.set_prepare()

            ckgen_dev.gen_res()


class UUUSheet(SheetRead):

    col_info = {
        "ROOT_NAME":    -1,
        "CKO_IDX":      -1,
        "ROOT Freq":    -1,
        "SelClk":       -1,
        "available":    -1,
        "prepare":      -1
    }

    def __init__(self, workbook, sheet_name, ckgen_sheet):
        super().__init__(workbook, sheet_name)
        self.ckgen_sheet = ckgen_sheet
        self.uuu_idx_list = {}
        self.uuu_cnt = 0

    def preprocessing(self, start_row):
        super().init_col_index_info(start_row)
        self.load_res_list(start_row + 1)

    def set_ckgen_idx(self, name):
        if name not in self.uuu_idx_list:
            self.uuu_idx_list[name] = self.uuu_cnt
            self.uuu_cnt += 1

    def get_ckgen_idx(self, name):
        return self.uuu_idx_list[name]

    def gen_res(self, start_row, platform):

        self.preprocessing(start_row)

        for i, item in enumerate(self.res_list):
            name = convert_root_name(item["ROOT_NAME"].lower())
            freq = convert_freq(item["SelClk"])
            self.set_ckgen_idx(name)

            if "available" in item and item["available"] == "N":
                continue

            ckgen_idx = self.get_ckgen_idx(name)
            cko_idx = int(item["CKO_IDX"])
            uuu_dev = UUUDev(name, freq, ckgen_idx, cko_idx)
            if "prepare" in item and item["prepare"] == "Y":
                uuu_dev.set_prepare()
            uuu_dev.gen_res()


class GatingSheet(SheetRead):

    col_info = {
        "Idx":          -1,
        "Name":         -1,
        "Source":       -1,
        "Type":         -1,
        "Enable":       -1,
        "available":    -1
    }

    def __init__(self, workbook, sheet_name, domain):
        super().__init__(workbook, sheet_name)
        self.domain = domain.lower()

    def preprocessing(self, start_row):
        super().init_col_index_info(start_row)
        self.load_res_list(start_row + 1)

    def gen_res(self, start_row, platform):

        self.preprocessing(start_row)

        for i, item in enumerate(self.res_list):

            if "available" in item and item["available"] == "N":
                continue

            idx = int(item["Idx"])
            name = item["Name"].lower()

            if item["Enable"] == "Y":
                enable = 1
            elif item["Enable"] == "N":
                enable = 2
            else:
                enable = 0

            if platform == 'kunlun':
                gating_dev = GatingDev(name, idx, self.domain, enable)
            elif platform == 'taishan':
                gating_dev = TaishanGatingDev(name, idx, self.domain, enable, item["Type"])
            else:
                raise Exception("Gating platform error: {}".format(platform))
            gating_dev.gen_res()
