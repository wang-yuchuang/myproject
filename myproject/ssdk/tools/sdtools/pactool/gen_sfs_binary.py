# -*- coding: utf-8 -*-
import sys
import os
sys.path.append(os.path.join(os.getcwd(), 'pactool'))
import json
import traceback
import getopt
import struct
from ctypes import create_string_buffer
from sfs_module import sfs_crc32

class GenSFS(object):

    def __init__(self):
        self.tag = 0x53465301
        # self.init_act_data = struct.pack("60x")
        self.init_act_data = create_string_buffer(60)
        self.xfer_data = struct.pack("16x")
        self.ip_data = struct.pack("16x")
        self.freq = 0
        self.sw_reset_info = 0  # 1bytes
        self.training_pattern = struct.pack("8x")

    def parse_value(self, arg):
        if sys.version_info[0] == 3 and isinstance(arg, str):
            return int(eval(str(arg)))
        elif sys.version_info[0] == 2 and (isinstance(arg, str) or isinstance(arg, unicode)):
            return int(eval(str(arg)))
        else:
            return int(arg)

    def generate_sfs(self, jason_file, sfs_file, normal_img_addr, backup_img_addr, third_img_addr):
        try:
            self.load_json(jason_file)
            buffer = struct.pack('<I'
                                 '60s'
                                 '16s'
                                 '16s'
                                 'B'
                                 '6x'   # change reserved[7] to reserved[6]
                                 'B'    # sw_reset_info, change 'reseraved' to 'sw_reset_info'
                                 '8s'
                                 'I'  # normal_img_addr
                                 'I'  # backup_img_addr
                                 'I',  # third_img_addr, change 'reseraved' to 'third_img_addr' 
                                 self.tag,
                                 self.init_act_data.raw,
                                 self.xfer_data,
                                 self.ip_data,
                                 self.freq,
                                 self.sw_reset_info,
                                 self.training_pattern,
                                 normal_img_addr,
                                 backup_img_addr,
                                 third_img_addr
                                 )

            crc32 = sfs_crc32(0, buffer, 124)
            # print(repr(buffer))
            # print("crc32=0x%X" % crc32)

            data = struct.pack('<124sI',
                               buffer,
                               crc32)

            image = open(sfs_file, 'wb')
            image.write(data)
            return True
        except ValueError as e:
            # error occurred.
            raise Exception("Failed to generate sfs file.")

    def load_json(self, jason_file):
        try:
            load_f = open(jason_file, 'r')
            obj = json.loads(load_f.read())

            # parse tag
            tag_obj = obj.get("tag")
            if tag_obj:
                self.tag = self.parse_value(tag_obj)

            # parse init_act_t
            init_act_t = obj.get("init_act_t")
            if init_act_t:
                count = 0
                for item in init_act_t:
                    struct.pack_into("BBHBB",
                                     self.init_act_data,
                                     count*6,
                                     self.parse_value(item["header"]),
                                     self.parse_value(item["command"]),
                                     self.parse_value(item["addr"]),
                                     self.parse_value(item["data0"]),
                                     self.parse_value(item["data1"]))
                    count += 1
            # print(repr(self.init_act_data.raw ))

            # parse xfer_attr_t
            xfer_obj = obj.get("xfer_attr_t")
            if xfer_obj:
                self.xfer_data = struct.pack("16B",
                                             self.parse_value(xfer_obj["cmd"]),
                                             self.parse_value(xfer_obj["cinst_type"]),
                                             self.parse_value(xfer_obj["caddr_type"]),
                                             self.parse_value(xfer_obj["cdata_type"]),
                                             self.parse_value(xfer_obj["caddr_size"]),
                                             self.parse_value(xfer_obj["cmode"]),
                                             self.parse_value(xfer_obj["cdummy_size"]),
                                             self.parse_value(xfer_obj["cflag"]),
                                             self.parse_value(xfer_obj["rsvd"][0]),
                                             self.parse_value(xfer_obj["rsvd"][1]),
                                             self.parse_value(xfer_obj["rsvd"][2]),
                                             self.parse_value(xfer_obj["rsvd"][3]),
                                             self.parse_value(xfer_obj["rsvd"][4]),
                                             self.parse_value(xfer_obj["rsvd"][5]),
                                             self.parse_value(xfer_obj["rsvd"][6]),
                                             self.parse_value(xfer_obj["rsvd"][7]),
                                             )
            # print(repr(self.xfer_data))

            # parse ip_settings_t
            ip_setting_obj = obj.get("ip_settings_t")
            if ip_setting_obj:
                self.ip_data = struct.pack("H14B",
                                           self.parse_value(ip_setting_obj["flags"]),
                                           self.parse_value(ip_setting_obj["rx_delay"]),
                                           self.parse_value(ip_setting_obj["tx_delay"]),
                                           self.parse_value(ip_setting_obj["csda"]),
                                           self.parse_value(ip_setting_obj["csdada"]),
                                           self.parse_value(ip_setting_obj["cseot"]),
                                           self.parse_value(ip_setting_obj["cssot"]),
                                           self.parse_value(ip_setting_obj["min_rx_win"]),
                                           self.parse_value(ip_setting_obj["rx_training_step"]),
                                           self.parse_value(ip_setting_obj["ctrl_misc"]),
                                           self.parse_value(ip_setting_obj["rsvd"][0]),
                                           self.parse_value(ip_setting_obj["rsvd"][1]),
                                           self.parse_value(ip_setting_obj["rsvd"][2]),
                                           self.parse_value(ip_setting_obj["rsvd"][3]),
                                           self.parse_value(ip_setting_obj["rsvd"][4])
                                           )
            # print(repr(self.ip_data))

            # parse freq
            freq_obj = obj.get("freq")
            if freq_obj:
                self.freq = self.parse_value(freq_obj)

            # parse sw_reset_info
            freq_obj = obj.get("sw_reset_info")
            if freq_obj:
                self.sw_reset_info = self.parse_value(freq_obj)

            # parse training_pattern
            training_obj = obj.get("training_pattern")
            if training_obj:
                self.training_pattern = struct.pack("8B",
                                                    self.parse_value(training_obj[0]),
                                                    self.parse_value(training_obj[1]),
                                                    self.parse_value(training_obj[2]),
                                                    self.parse_value(training_obj[3]),
                                                    self.parse_value(training_obj[4]),
                                                    self.parse_value(training_obj[5]),
                                                    self.parse_value(training_obj[6]),
                                                    self.parse_value(training_obj[7])
                                                    )
            # print(repr(self.training_pattern))

        except ValueError as e:
            # Unfortunately we can't easily get the line number where the
            # error occurred.
            raise Exception("Failed to Load jason file.")


def show_usage():
    print("gen_sfs_binary version: 1.21.50.1")
    print("usage: gen_sfs_binary.py <--json file> <--out file> [--normal_img_off offset] [--backup_img_off offset] [--third_img_off offset] [-h]")
    print("options:")
    print("--json file  Specify nor flash configuration")
    print("--out file   Output specify sfs binary file")
    print("--normal_img_off Normal image address")
    print("--backup_img_off Backup image address")
    print("--third_img_off  Third image address")
    print("-h           Help")
    print("e.g.: gen_sfs_binary.py --json sfs.json --out ./sfs.img")


def main(argv):
    """
    主函数
    :param argv: <--out file> [-h]
    :return: 0:pass; other:fail
    """
    exit_code = 1
    try:
        opts, args = getopt.getopt(argv, "-h", ["help", "json=", "out=", "normal_img_off=", "backup_img_off=", "third_img_off="])
        out_sfs_file = ""
        json_file = ""
        normal_img_addr = 0x7000
        backup_img_addr = 0x27000
        third_img_addr = 0x0
        for opt, arg in opts:
            if opt == '-h':
                show_usage()
                sys.exit(0)
            elif opt == "--out":
                out_sfs_file = arg
            elif opt == "--json":
                json_file = arg
            elif opt == "--normal_img_off":
                normal_img_addr = int(arg, 16)
                if len(arg) < 2 or not arg[:2].lower() == '0x':
                    normal_img_addr = int(arg, 10)
            elif opt == "--backup_img_off":
                backup_img_addr = int(arg, 16)
                if len(arg) < 2 or not arg[:2].lower() == '0x':
                    backup_img_addr = int(arg, 10)
            elif opt == "--third_img_off":
                third_img_addr = int(arg, 16)
                if len(arg) < 2 or not arg[:2].lower() == '0x':
                    third_img_addr = int(arg, 10)
        print("normal image address = 0x%08X" % normal_img_addr)
        print("backup image address = 0x%08X" % backup_img_addr)
        print(" third image address = 0x%08X" % third_img_addr)
        if len(out_sfs_file) == 0:
            show_usage()
            raise Exception("Not specify output sfs binary file info is empty.")

        if len(json_file) == 0:
            show_usage()
            raise Exception("Not input nor flash jason configuration file.")

        obj = GenSFS()
        if not obj.generate_sfs(json_file, out_sfs_file, normal_img_addr, backup_img_addr, third_img_addr):
            raise Exception("[Error] Failed to sfs binary file.")

    except Exception:
        print("[Error] %s" % traceback.format_exc())
    else:
        print("Successfully generated sfs binary file!")
        exit_code = 0

    return exit_code


if __name__ == '__main__':
    main(sys.argv[1:])
