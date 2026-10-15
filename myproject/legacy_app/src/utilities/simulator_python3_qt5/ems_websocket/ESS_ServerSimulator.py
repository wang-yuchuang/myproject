import pandas as pd
import numpy as np
import socket
import time
import threading

from PyQt5.QtWidgets import QVBoxLayout
from PyQt5.QtCore import QObject, pyqtSignal

from Gui import *
import hashlib
import base64
import json
import re

import sys
from pathlib import Path

from resource.reply_rs import reply_mapping
import resource.send_rs as send_packet
import resource.static_value as static
from ems_plan_gui import get_ems_plan_gui

from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure

# import matplotlib.pyplot as plt

# 将 resource 所在的目录添加到 sys.path
sys.path.append(str(Path(__file__).resolve().parent.parent))


# ===== Process Data =====
def process_data(raw_data_list, connection, output_file):
    while raw_data_list:
        if all(int(char, 16) == ord(expected_char) for expected_char, char in zip('GET', raw_data_list[:3])):
            process_handshake(raw_data_list, connection, output_file)
            raw_data_list = []
        else:
            required_convrt_data, mask_code, max_data, err = process_frame(raw_data_list)
            if err:
                break
            decoded_data = decode_frame(required_convrt_data, mask_code)
            process_decoded_frame(decoded_data, max_data, output_file, connection)
            raw_data_list = []


def process_handshake(raw_data_list, connection, output_file):
    # 解析握手请求
    handshake_list = [chr(int(i, 16)) for i in raw_data_list]
    handshake_list = ''.join(handshake_list).split('\r\n')

    # 获取接收到的WebSocket密钥
    rcvd_key = handshake_list[6].split(':')[1].strip()

    # 生成响应密钥
    websocket_key = create_websocket_key(rcvd_key)

    # 构造握手响应
    handshake_response = (
        'HTTP/1.1 101 Switching Protocols\r\n'
        'Upgrade: websocket\r\n'
        'Connection: Upgrade\r\n'
        f'Sec-WebSocket-Accept: {websocket_key}\r\n'
        'Sec-WebSocket-Protocol: ESS Websocket\r\n\r\n'
    )

    # 发送握手响应
    connection.sendall(handshake_response.encode())
    print('Handshake:')
    print(handshake_response)
    # output_file.write('Handshake Response sent\n')
    time.sleep(2)
    # print(data)


def process_frame(raw_data_list):
    if (raw_data_list[0] == '0x1' or raw_data_list[0] == '0x80' or raw_data_list[0] == '0x81') and raw_data_list[
        1] != '0xfe':
        return process_normal_frame(raw_data_list)
    elif raw_data_list[1] == '0xfe':
        return process_long_frame(raw_data_list)
    # elif raw_data_list[0] == '0x89' and raw_data_list[1] == '0x80':
    #     print('Recv Ping Cmd: ', raw_data_list)
    #     return None, None, None, True
    else:
        print('Wrong packet')
        print('raw_data_list: ', raw_data_list)
        return None, None, None, True


def process_normal_frame(raw_data_list):
    max_data = int(raw_data_list[1], 16) - 128
    required_convrt_data = raw_data_list[6:]
    remove_data = raw_data_list[:6]
    mask_code = [chr(int(i, 16)) for i in remove_data[2:]]
    raw_data_list[:] = [i for i in raw_data_list if i not in remove_data]
    processed_data = required_convrt_data[:max_data]
    return processed_data, mask_code, max_data, False


def process_long_frame(raw_data_list):
    secondbyte = raw_data_list[3][2:].zfill(2)
    max_data_1 = f"{raw_data_list[2]}{secondbyte}"
    max_data = int(max_data_1, 16)
    required_convrt_data = raw_data_list[8:]
    remove_data = raw_data_list[:8]
    mask_code = [chr(int(i, 16)) for i in remove_data[4:]]
    raw_data_list[:] = [i for i in raw_data_list if i not in remove_data]
    processed_data = required_convrt_data[:max_data]
    return processed_data, mask_code, max_data, False


def decode_frame(required_convrt_data, mask_code):
    decoded_data = []
    index = 0
    for i in required_convrt_data:
        test = int(i, 16) ^ ord(mask_code[index])
        test = chr(test)

        if test not in ['\n', '\t']:
            decoded_data.append(test)

        index = (index + 1) % 4  # 简化索引递增和重置逻辑
    return decoded_data


def process_decoded_frame(decoded_data, max_data, output_file, connection):
    # if len(decoded_data) >= max_data and decoded_data[-1] == '}':
    if decoded_data[-1] == '}':
        decoded_data_str = ''.join(decoded_data)
        dict_decoded_data = json.loads(decoded_data_str)
        print('Received Packet Data: ', dict_decoded_data)
        # output_file.write(f'Packet_Recv: {decoded_data_str}\n')

        if len(decoded_data_str) > 1:
            check_for_reply, unique_key = dict_decoded_data['id'], dict_decoded_data['uniqKey']
            if check_for_reply[0] == 'Q':
                refresh_received_packet2show(check_for_reply, dict_decoded_data['content'])
                send_cp_reply(check_for_reply, connection, output_file, unique_key)

                # 检测系统状态
                # judge_reply_content(check_for_reply, dict_decoded_data)

                global autotest_flag
                # 检验自动测试回复报文
                if autotest_flag == 0:
                    print('Not Auto Test')
                elif autotest_flag == 1:
                    running_autotest(check_for_reply, dict_decoded_data)
                    if check_for_reply == 'Q105':
                        # output_file.write(f'Q105: {dict_decoded_data}\n')
                        global q105_cab_data, q105_grp_data
                        q105_cab_data, q105_grp_data = save_q105_data(dict_decoded_data['content'], q105_cab_data,
                                                                      q105_grp_data)
                    elif check_for_reply == 'Q107':
                        global q107_data
                        q107_data = save_q107_data(dict_decoded_data['content']['ac_metervalue'][1], q107_data)
                        print('save q107!')
                    # elif check_for_reply == 'Q202':
                    #     global q202_meter_data, q202_grp_data
                    #     q202_meter_data, q202_grp_data = save_q202_data(dict_decoded_data['content'], q202_meter_data, q202_grp_data)
                elif autotest_flag == 2:
                    save_autotest_packet(q105=True, q107=True, q202=False)
                    # global q105_cab_data, q105_grp_data
                    # q105_cab_data = q105_cab_data

                    q105_cab_test_result, q105_grp_test_result = judge_q105_data(q105_cab_data, q105_grp_data)
                    # print(f'Cab Test Result: {q105_cab_test_result}')
                    # print(f'Grp1 Test Result: {q105_grp_test_result[0][1]}')
                    # print(f'Grp2 Test Result: {q105_grp_test_result[1][1]}')
                    # print(f'Grp3 Test Result: {q105_grp_test_result[2][1]}')
                    # print(f'Grp4 Test Result: {q105_grp_test_result[3][1]}')

                    q107_result = judge_q107_data(q107_data)
                    print(f'q107_result: {q107_result}')
                    # global q202_meter_test_result, q202_grp_test_result
                    # q202_meter_test_result, q202_grp_test_result = judge_q202_data(q202_meter_data, q202_grp_data)

                    if q105_cab_test_result or q105_grp_test_result[0][1] or q105_grp_test_result[1][1] or \
                            q105_grp_test_result[2][1] or \
                            q105_grp_test_result[3][1]:
                        autotest_flag = 4  # 至少有一个测试失败
                    else:
                        autotest_flag = 3

                    # global autotest_flag
                    # autotest_flag = 0

            elif check_for_reply == 'R503':
                global ems_plan
                ems_plan = dict_decoded_data
    else:
        print('Packet Not Complete!')


def running_autotest(id, dict_decoded_data):
    # def judge_total_info(plan_info, read_info):
    #     print('judge_total_info')
    #     status = False
    #     # 判断自动充放电是否正常
    #     set_work_mode = plan_info['batt_workmode']
    #     real_work_mode = read_info['ac_workstat']
    #     if set_work_mode == 0 and real_work_mode != 3:
    #         print('Auto Discharge Err!')
    #         status = True
    #     elif set_work_mode == 1 and real_work_mode != 2:
    #         print('Auto Charge Err!')
    #         status = True
    #     else:
    #         print('Auto Mode Normal.')
    #
    #     # 判断柜内电表是否符合功率限制
    #     set_pwr_limit = plan_info['actv_pwr_lmt_rate']
    #     real_pwr = read_info['kw_meter_cab']
    #     if abs(real_pwr - set_pwr_limit) > 5:
    #         print('Pwr Limit Error!')
    #         status = True
    #     else:
    #         print('Pwr Limit Normal!')
    #     return status
    #
    # def judge_single_info(plan_info, read_info):
    #     print('judge_single_info')
    #     status = False
    #     single_info = read_info['branch_grp_data']
    #     # 判断单柜PCS工作模式
    #     set_work_mode = plan_info['batt_workmode']
    #     n = len(single_info)
    #     for i in range(n):
    #         real_cab_work_mode = single_info[i]['acdc_workstat']
    #         if set_work_mode == 0 and real_cab_work_mode != 2:
    #             print(f'No.{i + 1} Cab Auto Discharge Err!')
    #             status = True
    #         elif set_work_mode == 1 and real_cab_work_mode != 1:
    #             print(f'No.{i + 1} Cab Auto Charge Err!')
    #             status = True
    #         else:
    #             print(f'No.{i + 1} Cab Auto Mode Normal.')
    #     return status

    ems_plan_content = autotest_plan['content']
    decoded_content = dict_decoded_data['content']

    start_time_1st = ems_plan_content['ems_plan_cfg'][0]['start_time']
    start_time_2nd = ems_plan_content['ems_plan_cfg'][1]['start_time']

    TIME_GAP = int(ui.lineEdit_autotest_time.text())
    end_time = time_converter(start_time_2nd, TIME_GAP)

    time_now = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())
    ui.text_autotest_time.setText(
        f'(1){start_time_1st}->{start_time_2nd}\n(2){start_time_2nd}->{end_time}\n')

    if start_time_1st <= time_now <= start_time_2nd:
        print('1st Step.')
        # ui.text_autotest_time.setText(f'Step 1\nStart: {start_time_1st}\nEnd: {start_time_2nd}')
        # if decoded_content['ac_workstat'] == 3:  # 系统响应阶段不判断
        #     total_res = judge_total_info(ems_plan_content['ems_plan_cfg'][0], decoded_content)
        #     single_res = judge_single_info(ems_plan_content['ems_plan_cfg'][0], decoded_content)
        #     if total_res or single_res:
        #         print('First Period not pass!')
    elif start_time_2nd <= time_now <= end_time:
        print('2nd Step.')
        # ui.text_autotest_time.setText(f'Step 2\nStart: {start_time_2nd}\nEnd: {end_time}')
        # if decoded_content['ac_workstat'] == 2:  # 系统响应阶段不判断
        #     total_res = judge_total_info(ems_plan_content['ems_plan_cfg'][1], decoded_content)
        #     single_res = judge_single_info(ems_plan_content['ems_plan_cfg'][1], decoded_content)
        #     if total_res or single_res:
        #         print('Second Period not pass!')
    elif time_now > end_time:
        global autotest_flag
        autotest_flag = 2

        print('test ends!')
    else:
        print('err time!')


# ===== Send Packet ======
def send_ws_packet(packet, connection, output_file):
    # 打印原始数据包
    # print('')
    # print('')
    print('Raw Packet:', packet)

    # 将字符串转换为对应的十六进制表示，并存储在 check_l_list 中
    check_l_list = []
    for i in packet:
        raw = hex(ord(i))
        check_l_list.append(raw)

    # 计算数据包长度
    pkt_len = len(check_l_list)

    # 定义 WebSocket 的 opcode，使用字节串表示 opcode
    opcode = b'\x81'

    # 根据数据包长度选择适当的表示方式
    if pkt_len > 125:
        # 如果数据包长度大于125，使用 16 位表示长度
        second_byte = b'\x7E'
        pkt_len_bytes = pkt_len.to_bytes(2, byteorder='big')
        packet = opcode + second_byte + pkt_len_bytes + packet.encode('utf-8')
    else:
        # 如果数据包长度小于等于125，使用 8 位表示长度
        pkt_len_byte = pkt_len.to_bytes(1, byteorder='big')
        packet = opcode + pkt_len_byte + packet.encode('utf-8')

    # 打印要发送的 WebSocket 数据包
    print('Packet to Send:', packet)

    # 将数据包发送到连接
    connection.sendall(packet)

    # 将发送的数据包写入输出文件
    # output_file.write('Packet_Tran:')
    # output_file.write(packet.decode('utf-8', errors='replace'))
    # output_file.write('\n')

    # 等待一段时间，可根据具体情况调整，以确保接收方有足够的时间处理数据
    time.sleep(0.5)


def send_ws_packet_response(reply_cmd, unique_key, content, connection, output_file):
    # 使用正则表达式匹配数字部分
    match = re.search(r'\d+', unique_key)
    # 提取匹配到的数字部分
    if match:
        unique_key = match.group()
        # print("Unique Key: ", unique_key)
    else:
        print("No Unique Key found.")

    packet = {
        "id": reply_cmd,
        "uniqKey": unique_key,
        "content": content
    }
    packet = json.dumps(packet)
    # boot_reply = boot_reply.replace(" ", "")
    print('Send Response:', packet)
    # print('')
    # print('')

    pkt_len = len(packet)
    opcode = b'\x81'  # 使用字节串表示 opcode

    if pkt_len > 125:
        second_byte = b'\x7E'
        pkt_len_bytes = pkt_len.to_bytes(2, byteorder='big')
        packet = opcode + second_byte + pkt_len_bytes + packet.encode('utf-8')
    else:
        pkt_len_byte = pkt_len.to_bytes(1, byteorder='big')
        packet = opcode + pkt_len_byte + packet.encode('utf-8')

    # test = [hex(b) for b in packet]  # 使用列表推导简化测试列表的构建

    connection.sendall(packet)

    # output_file.write('Packet_Tran:')
    # # output_file.write(packet)  # 将字节串解码为 UTF-8 编码的字符串
    # output_file.write(packet.decode('utf-8', errors='replace'))
    # # output_file.write(packet.decode('utf-8'))  # 将字节串解码为 UTF-8 编码的字符串并写入文件
    # output_file.write('\n')

    time.sleep(0.1)


def send_ems_ctl(ui, connection, output_file):
    if ui.checkBox_reademsplan.isChecked():
        cmd = json.dumps(send_packet.read_ems_plan)
        send_ws_packet(cmd, connection, output_file)
        ui.checkBox_reademsplan.setChecked(False)
    if ui.checkBox_setemsplan.isChecked():
        if not ems_plan:
            print('')
            print('Read EMS Plan First!')
        else:
            new_ems_plan = get_ems_plan_gui(ems_plan)
            new_ems_plan['id'] = 'Q504'
            new_ems_plan['content']['ems_plan_ver'] += 1
            cmd = json.dumps(new_ems_plan)
            send_ws_packet(cmd, connection, output_file)
        ui.checkBox_setemsplan.setChecked(False)
    if ui.checkBox_defaultplan.isChecked():
        cmd = json.dumps(send_packet.default_ems_plan)
        send_ws_packet(cmd, connection, output_file)
        ui.checkBox_defaultplan.setChecked(False)
    if ui.checkBox_autotest.isChecked():
        global autotest_flag, autotest_plan
        autotest_flag = 1

        # 初始化 DataFrame
        global q105_cab_data, q105_grp_data
        global q107_data
        global q202_meter_data, q202_grp_data

        q105_cab_data = pd.DataFrame()
        q105_grp_data = {}

        q107_data = pd.DataFrame()

        q202_meter_data = pd.DataFrame()
        q202_grp_data = {}

        autotest_plan = send_packet.autotest_ems_plan

        get_time_now = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())

        start_time_1st = time_converter(get_time_now, 0)

        TIME_GAP = int(ui.lineEdit_autotest_time.text())
        print(TIME_GAP)

        start_time_2nd = time_converter(start_time_1st, TIME_GAP)

        autotest_plan['content']['ems_plan_cfg'][0]['start_time'] = start_time_1st
        autotest_plan['content']['ems_plan_cfg'][0]['interval'] = TIME_GAP * 60
        autotest_plan['content']['ems_plan_cfg'][1]['start_time'] = start_time_2nd
        autotest_plan['content']['ems_plan_cfg'][1]['interval'] = TIME_GAP * 60

        autotest_plan['content']['ems_plan_ver'] = int(time.strftime('%M%S', time.localtime()))

        if ui.comboBox_autotest.currentIndex() == 0:
            autotest_plan['content']['ems_plan_cfg'][0]['batt_workmode'] = 0
            autotest_plan['content']['ems_plan_cfg'][1]['batt_workmode'] = 1
        elif ui.comboBox_autotest.currentIndex() == 1:
            autotest_plan['content']['ems_plan_cfg'][0]['batt_workmode'] = 1
            autotest_plan['content']['ems_plan_cfg'][1]['batt_workmode'] = 0
        else:
            print('error comboBox_autotest')

        active_pwr = int(ui.lineEdit_autotest_activepwr.text())
        autotest_plan['content']['ems_plan_cfg'][0]['actv_pwr_lmt_rate'] = active_pwr
        autotest_plan['content']['ems_plan_cfg'][1]['actv_pwr_lmt_rate'] = active_pwr

        reactive_pwr = int(ui.lineEdit_autotest_reactivepwr.text())
        autotest_plan['content']['ems_plan_cfg'][0]['reactv_pwr_lmt_rate'] = reactive_pwr
        autotest_plan['content']['ems_plan_cfg'][1]['reactv_pwr_lmt_rate'] = reactive_pwr

        batt_pwr = int(ui.lineEdit_autotest_battpwr.text())
        autotest_plan['content']['ems_plan_cfg'][0]['battgrp_pwr_lmt_rate'] = batt_pwr
        autotest_plan['content']['ems_plan_cfg'][1]['battgrp_pwr_lmt_rate'] = batt_pwr

        cmd = json.dumps(autotest_plan)
        send_ws_packet(cmd, connection, output_file)

        ui.checkBox_autotest.setChecked(False)
    if ui.checkBox_q411.isChecked():
        cmd = json.dumps(send_packet.q411)
        send_ws_packet(cmd, connection, output_file)
        ui.checkBox_q411.setChecked(False)
    if ui.checkBox_q412.isChecked():
        cmd = json.dumps(send_packet.q412)
        send_ws_packet(cmd, connection, output_file)
        ui.checkBox_q412.setChecked(False)


def send_cp_reply(check_for_reply, connection, output_file, key):
    # print('request: Check_for_reply', check_for_reply)

    # 检查 Check_for_reply 是否在映射中
    if check_for_reply in reply_mapping:  # 去掉 '{"id":' 部分比较
        cmd, reply_content, set_time = reply_mapping[check_for_reply]

        # 如果需要时间戳，设置时间
        if set_time:
            get_time_now = time.localtime()
            send_time = str(get_time_now[0]) + '-' + str(get_time_now[1]) + '-' + str(get_time_now[2]) + 'T' + str(
                get_time_now[3]) + ':' + str(get_time_now[4]) + ':' + str(get_time_now[5]) + 'Z'
            reply_content['time'] = send_time

        # 发送响应
        send_ws_packet_response(reply_cmd=cmd, unique_key=key, content=reply_content, connection=connection,
                                output_file=output_file)


# ===== Refresh Data =====
def refresh_received_packet2show(id, content):
    if id == 'Q105':
        global ac_volts, ac_currs, kw_meter_grid, kw_meter_cab, kw_acload
        global batt_workstat, voltage, current, soc, acdc_workstat, acdc_volt, acdc_curr, acdc_ext_volt

        ac_volts = content['ac_volts']
        ac_currs = content['ac_currs']

        kw_meter_grid = content['kw_meter_grid']
        kw_meter_cab = content['kw_meter_cab']
        kw_acload = content['kw_acload']

        # 检查是否存在所需的键
        if 'branch_grp_data' in content:
            grp_data = content['branch_grp_data']

            # 检查grp_data的长度，防止索引越界
            if len(grp_data) >= 4:
                for i in range(4):
                    # 使用get方法来避免KeyError
                    batt_workstat[i] = grp_data[i].get('batt_workstat', None)
                    voltage[i] = grp_data[i].get('batt_volt', None)
                    current[i] = grp_data[i].get('batt_curr', None)
                    soc[i] = grp_data[i].get('batt_soc', None)

                    acdc_workstat[i] = grp_data[i].get('acdc_workstat', None)
                    acdc_volt[i] = grp_data[i].get('acdc_volt', None)
                    acdc_curr[i] = grp_data[i].get('acdc_curr', None)
                    acdc_ext_volt[i] = grp_data[i].get('acdc_ext_volt', None)
            else:
                print("Warning: 'branch_grp_data' does not contain enough data.")
        else:
            print("Warning: 'content' does not have 'branch_grp_data'.")
    # if id == 'Q107':
    #     global grid_meter_active_pwr
    #     global grid_meter_acvolts, grid_meter_accurrs
    #     received_value = content['ac_metervalue']
    #     if received_value[1]['data_from'] == static.Userside_AC_Meter:
    #         acmeter_value = received_value[1]
    #         grid_meter_active_pwr = acmeter_value['active_power']
    #         grid_meter_acvolts = acmeter_value['phase_volts']
    #         grid_meter_accurrs = acmeter_value['phase_currs']


def refresh_all_info(ui):
    refresh_batt_info(ui)
    refresh_pcs_info(ui)
    refresh_cab_info(ui)
    # refresh_abu_info(ui)
    # refresh_meter_info(ui)
    refresh_autotest_result(ui)


def refresh_batt_info(ui):
    # 定义状态映射
    status_map = {0: '未安装', 1: '空闲', 2: '充电', 3: '放电'}
    # 通过循环设置文本
    for i in range(4):
        status = status_map.get(batt_workstat[i], "未知状态")
        text = f'{status}\n{voltage[i]}V {current[i]}A {soc[i]}%'
        getattr(ui, f'text_battinfo{i + 1}').setText(text)


def refresh_cab_info(ui):
    text = f'{ac_volts}V\n{ac_currs}A\n{kw_meter_cab}kW'
    ui.text_cabinfo.setText(text)


"""
def refresh_abu_info(ui):
    print('Refreshing')
"""


def refresh_pcs_info(ui):
    # 定义状态映射
    status_map = {0: '空闲', 1: '并网整流', 2: '并网逆变', 3: '离网逆变'}
    # 通过循环设置文本
    for i in range(4):
        status = status_map.get(acdc_workstat[i], "未知状态")
        # text = f'{status}\n{acdc_curr[i]}A\n{acdc_volt[i]}V {acdc_ext_volt}V(ext)%'
        text_dc = f'{acdc_curr[i]}A\n{acdc_volt[i]}V'
        text_ac = f'{status}'
        getattr(ui, f'text_dc_pcs{i + 1}').setText(text_dc)
        getattr(ui, f'text_ac_pcs{i + 1}').setText(text_ac)


"""
def refresh_meter_info(ui):
    print('Refreshing')
    # 'PCS输出:{acdc_volt[i]}V {acdc_curr[i]}A\nPCS外侧:{acdc_ext_volt[i]}V'
"""


def refresh_autotest_result(ui):
    # 定义状态映射
    result_map = {0: '测试未开始', 1: '测试中', 2: '校验结果', 3: 'Pass', 4: 'Fail！'}
    autotest_result = result_map.get(autotest_flag)
    text = f'{autotest_result}'
    ui.text_autotest.setText(text)
    if autotest_result == 'Fail!' or autotest_result == 'Pass':
        ui.text_autotest.setStyleSheet("color: red; border: 1px solid black;")
    else:
        ui.text_autotest.setStyleSheet("color: black; border: 1px solid black;")


# ===== Judge Result =====
def save_q105_data(value, q105_cab_data, q105_grp_data):
    # 更新主数据 DataFrame
    main_data = {k: v for k, v in value.items() if k != 'branch_grp_data'}
    q105_cab_data = pd.concat([q105_cab_data, pd.DataFrame([main_data])], ignore_index=True)

    # 更新分支组数据 DataFrame
    for idx, item in enumerate(value['branch_grp_data']):
        df_name = f'df_branch_grp_{idx}'
        new_df = pd.DataFrame([item])
        if df_name not in q105_grp_data:
            # 如果 DataFrame 还不存在，创建它
            q105_grp_data[df_name] = new_df
        else:
            # 否则，更新现有的 DataFrame
            q105_grp_data[df_name] = pd.concat([q105_grp_data[df_name], new_df], ignore_index=True)

    return q105_cab_data, q105_grp_data


def save_q107_data(value, q107_data):
    q107_data = pd.concat([q107_data, pd.DataFrame([value])], ignore_index=True)
    return q107_data


def save_q202_data(value, q202_meter_data, q202_grp_data):
    # 更新主数据 DataFrame
    main_data = {k: v for k, v in value.items() if k != 'mppt_grp_output_kwh' and k != 'batt_grp_chrgdischrgkwh'}
    q202_meter_data = pd.concat([q202_meter_data, pd.DataFrame([main_data])], ignore_index=True)

    # 更新分支组数据 DataFrame
    for idx, item in enumerate(value['batt_grp_chrgdischrgkwh']):
        df_name = f'df_branch_grp_{idx}'
        new_df = pd.DataFrame([item])
        if df_name not in q202_grp_data:
            # 如果 DataFrame 还不存在，创建它
            q202_grp_data[df_name] = new_df
        else:
            # 否则，更新现有的 DataFrame
            q202_grp_data[df_name] = pd.concat([q202_grp_data[df_name], new_df], ignore_index=True)

    return q202_meter_data, q202_grp_data


def judge_q105_data(q105_cab_data, q105_grp_data):
    # 对数据进行切片 分析连续的充放电数据
    def cut_segment(input_data, column_name, dischg_label, chg_label, min_len):
        dischg_segment = pd.DataFrame()
        chg_segment = pd.DataFrame()
        # Create a new column 'segment' to identify different segments of consecutive identical 'ac_workstat' values
        # The 'shift' function is used to compare with the previous row. When the value changes, the cumsum function
        # will treat this as a new segment.
        input_data['segment'] = (input_data[f'{column_name}'] != input_data[f'{column_name}'].shift(1)).cumsum()

        # Dictionary to store dataframes for each segment
        segmented_dfs = {}

        # Split the dataframe into separate dataframes for each segment
        for segment_number in input_data['segment'].unique():
            segmented_dfs[segment_number] = input_data[input_data['segment'] == segment_number]
        for i in segmented_dfs:
            if segmented_dfs[i][f'{column_name}'].iloc[0] == dischg_label and len(
                    segmented_dfs[i]) > min_len:
                dischg_segment = segmented_dfs[i]
                ui.text_autotest_res1_1.setText('自动充电控制正常')
            else:
                ui.text_autotest_res1_1.setText('自动充电控制异常')
                ui.text_autotest_res1_1.setStyleSheet("color: red;")
            if segmented_dfs[i][f'{column_name}'].iloc[0] == chg_label and len(segmented_dfs[i]) > min_len:
                chg_segment = segmented_dfs[i]
                ui.text_autotest_res1_2.setText('自动放电控制正常')
            else:
                ui.text_autotest_res1_2.setText('自动放电控制异常')
                ui.text_autotest_res1_2.setStyleSheet("color: red;")
        return dischg_segment, chg_segment

    # 判断电源柜整体信息
    def judge_cab_data(q105_cab_data, plan_info):
        cab_re = False

        # 数据切片
        dischg_segment, chg_segment = cut_segment(q105_cab_data, column_name='ac_workstat', dischg_label=3, chg_label=2,
                                                  min_len=5)

        # 判断ac电压
        # func

        # 判断ac电流
        # func

        # 判断电表数据
        dischg_pwr = abs(dischg_segment['kw_meter_cab']).max()
        dischg_pwr_plan = plan_info['ems_plan_cfg'][0]['actv_pwr_lmt_rate']

        chg_pwr = abs(chg_segment['kw_meter_cab']).max()
        chg_pwr_plan = plan_info['ems_plan_cfg'][1]['actv_pwr_lmt_rate']

        if abs(dischg_pwr - dischg_pwr_plan) > 50:
            cab_re = True
            print('Discharge pwr err!')
            ui.text_autotest_res2_1.setText('自动充放电功率异常')
            ui.text_autotest_res2_1.setStyleSheet("color: red;")
        else:
            ui.text_autotest_res2_1.setText('自动充放电功率正常')
        if abs(chg_pwr - chg_pwr_plan) > 50:
            cab_re = True
            print('Charge pwr err!')
            ui.text_autotest_res2_2.setText('自动充放电功率异常')
            ui.text_autotest_res2_2.setStyleSheet("color: red;")
        else:
            ui.text_autotest_res2_2.setText('自动充放电功率正常')

        return cab_re

    # 判断电池柜单簇信息
    def judge_grp_data(single_grp_data, plan_info):
        grp_re = False

        grp_idx = single_grp_data['branch_grp_idx'].iloc[0] + 1
        # 数据切片
        dischg_segment, chg_segment = cut_segment(single_grp_data, column_name='batt_workstat', dischg_label=3,
                                                  chg_label=2,
                                                  min_len=5)

        if len(dischg_segment) != 0:
            # 判断平均功率(kW)
            dischg_pwr_mean = np.mean(abs(dischg_segment['batt_volt'] * dischg_segment['batt_curr'])) / 1000
            dischg_pwr_plan = plan_info['ems_plan_cfg'][0]['battgrp_pwr_lmt_rate']
            if abs(dischg_pwr_mean - dischg_pwr_plan) > 50:
                grp_re = True
                print(f'No.{grp_idx}: Discharge pwr err!')
        else:
            # grp_re = True
            print(f'No.{grp_idx}: No Discharge Segment')

        if len(chg_segment) != 0:
            chg_pwr_mean = np.mean(abs(chg_segment['batt_volt'] * chg_segment['batt_curr'])) / 1000
            chg_pwr_plan = plan_info['ems_plan_cfg'][1]['battgrp_pwr_lmt_rate']
            if abs(chg_pwr_mean - chg_pwr_plan) > 50:
                grp_re = True
                print(f'No.{grp_idx}: Charge pwr err!')
        else:
            # grp_re = True
            print(f'No.{grp_idx}: No Charge Segment')

        return grp_re

    ems_plan_content = autotest_plan['content']

    q105_cab_test_result = judge_cab_data(q105_cab_data, ems_plan_content)

    q105_grp_test_result = []
    for i in range(len(q105_grp_data)):
        result = judge_grp_data(q105_grp_data[f'df_branch_grp_{i}'], ems_plan_content)
        q105_grp_test_result.append([i + 1, result])  # [优化] 使用 append 添加索引和结果

    return q105_cab_test_result, q105_grp_test_result


def judge_q107_data(q107_data):
    charge_kwh = q107_data['kwh_reading'].iloc[-1] - q107_data['kwh_reading'].iloc[0]
    discharge_kwh = q107_data['neg_kwh_reading'].iloc[-1] - q107_data['neg_kwh_reading'].iloc[0]
    ui.text_autotest_res4_1.setText(f'充电量: {charge_kwh} kWh')
    ui.text_autotest_res4_2.setText(f'放电量: {discharge_kwh} kWh')
    return False

def judge_q202_data(q202_meter_data, q202_grp_data):
    None


def save_autotest_packet(q105=False, q107=False, q202=False):
    if q105:
        q105_cab_data.to_csv('q105_cab_data.csv', index=False)
        grp_1 = q105_grp_data['df_branch_grp_0']
        grp_2 = q105_grp_data['df_branch_grp_1']
        grp_3 = q105_grp_data['df_branch_grp_2']
        grp_4 = q105_grp_data['df_branch_grp_3']
        grp_1.to_csv('q105_grp_1.csv', index=False)
        grp_2.to_csv('q105_grp_2.csv', index=False)
        grp_3.to_csv('q105_grp_3.csv', index=False)
        grp_4.to_csv('q105_grp_4.csv', index=False)
    if q107:
        q107_data.to_csv('q107_data.csv', index=False)
    if q202:
        q202_meter_data.to_csv('q202_meter_data.csv', index=False)

"""
def judge_batt_cab_status(content):
    def inspect(num, single_data):
        if single_data['comm_statu'] == 1:
            print(f"No.{num} Comm Err!")
        if single_data['flood_alarm'] == 1:
            print(f"No.{num} Flood Err!")
        # Complete Other Err

    batt_cab_data = content['batt_cab_data']
    n = len(batt_cab_data)
    for i in range(n):
        inspect(i, batt_cab_data[i])


def judge_bms_status(content):
    def inspect(num, single_data):
        if single_data['comm_statu'] == 1:
            print("Comm Err!")
        if single_data['flood_alarm'] == 1:
            print("Flood Err!")
        # Complete Other Err

    bms_data = content['batt_cab_data']
    n = len(bms_data)
    for i in range(n):
        inspect(i, bms_data[i])


def judge_energy_diagram_status(content):
    def inspect(num, single_data):
        if single_data['comm_statu'] == 1:
            print("Comm Err!")
        if single_data['flood_alarm'] == 1:
            print("Flood Err!")

    single_grp_data = content['branch_grp_data']
    n = len(single_grp_data)
    for i in range(n):
        inspect(i, single_grp_data[i])


def judge_reply_content(id, dict_decoded_data):
    content = dict_decoded_data['content']
    if id == 'Q101':
        if content['cp_status'] != 0 or content['cp_fault'] != 0:
            print(f"cp status: {content['cp_status']}")
            print(f"cp fault: {content['cp_fault']}")
        else:
            print("System status normal.")

    # elif id == 'Q102':
    #     judge_batt_cab_status(content)
    #
    # elif id == 'Q103':
    #     judge_batt_cab_status(content)

    # elif id == 'Q105':
    #     judge_energy_diagram_status(content)
"""


# ===== Plot Realtime Data =====
class Plotter(QObject):
    update_gui_signal = pyqtSignal()


def realtime_plot(ui):
    # 检查布局是否已存在
    if ui.frame_realtimeplot.layout() is None:
        layout = QVBoxLayout()
        ui.frame_realtimeplot.setLayout(layout)
    else:
        layout = ui.frame_realtimeplot.layout()

    # 检查画布是否已存在
    if layout.count() == 0:
        figure = Figure()
        canvas = FigureCanvas(figure)
        layout.addWidget(canvas)
    else:
        canvas = layout.itemAt(0).widget()
        figure = canvas.figure

    # 数据处理
    global batt_pwr_data, acload_pwr_data, cab_pwr_data, grid_pwr_data

    # Batt Power
    if kw_meter_cab:
        batt_pwr_data.append(kw_meter_cab)
    else:
        batt_pwr_data.append(0)

    # Grid Meter
    if kw_meter_grid:
        grid_pwr_data.append(kw_meter_grid)
    else:
        grid_pwr_data.append(0)

    # AC Load
    if kw_acload:
        acload_pwr_data.append(kw_acload)
    else:
        acload_pwr_data.append(0)

    # batt_pwr = 0
    # for i in range(4):
    #     batt_pwr += voltage[i] * current[i] / 1000
    # batt_pwr_data.append(batt_pwr)

    # Cab Meter
    # ac_pwr = 0
    # if ac_volts and ac_currs != []:
    #     for i in range(3):
    #         ac_pwr += ac_volts[i] * ac_currs[i] / 1000
    # # print(ac_volts)
    # # print(ac_currs)
    # acload_pwr_data.append(ac_pwr)

    # grid_pwr = 0
    # if grid_meter_acvolts and grid_meter_accurrs != []:
    #     for i in range(3):
    #         grid_pwr += grid_meter_acvolts[i] * grid_meter_accurrs[i] / 1000
    # print(ac_volts)
    # print(ac_currs)
    # grid_pwr_data.append(grid_pwr)

    # 清除旧图表并绘制新图表
    figure.clear()
    ax = figure.add_subplot(111)
    # ax.plot(batt_pwr_data, label='Batt Caculated Power')
    ax.plot(cab_pwr_data, label='Batt Power')
    ax.plot(acload_pwr_data, label='AC Load Power')
    ax.plot(grid_pwr_data, label='Grid Power')
    ax.set_xlabel('time')
    ax.set_ylabel('kW')
    ax.set_title('Real Time Power')

    # 显示图例，并设置图例在图形外的左侧
    # ax.legend(loc='center left', bbox_to_anchor=(-0.3, 0.5))
    ax.legend()

    # 调整子图参数，为图例腾出空间
    # plt.subplots_adjust(left=0.3)
    canvas.draw()


# ===== Tool Function =====
def create_websocket_key(cp_key):
    ws_key = '258EAFA5-E914-47DA-95CA-C5AB0DC85B11'
    sha_1 = cp_key + ws_key
    sha_1 = hashlib.sha1(sha_1.encode()).digest()
    sha_1_base64 = base64.b64encode(sha_1)
    return sha_1_base64


def time_converter(input_str, gap_minute):
    time_tuple = time.strptime(input_str, '%Y-%m-%d %H:%M:%S')
    # 将时间元组转换为时间戳
    timestamp = time.mktime(time_tuple)
    # 增加分钟数
    timestamp += gap_minute * 60
    # 将更新的时间戳转换回时间元组，然后格式化为字符串
    out_str = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(timestamp))
    return out_str


def initialize_socket(ui):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_ip = ui.lineEdit_serverip.text()
    server_port = int(float(ui.lineEdit_serverport.text()))
    server_address = (server_ip, server_port)
    print('Starting up on %s port %s' % server_address, file=sys.stderr)
    sock.bind(server_address)
    sock.listen(1)
    print('Waiting for a connection...')
    return sock


def create_log_filename(ui):
    get_time_now = time.strftime('%Y%m%d_%H%M%S', time.localtime())
    # file_name = ui.lineEdit.text() + '_' + get_time_now
    file_name = f'websocket_{get_time_now}'
    return f"./outfile/{file_name}.txt"


# ===== Main Application =====
def main_function(plotter):
    global ui, run_flag
    sock = initialize_socket(ui)
    connection, client_address = sock.accept()
    print('TCP:', client_address)

    try:
        print('Connection from', client_address, file=sys.stderr)
        new_path = create_log_filename(ui)

        while run_flag:
            refresh_all_info(ui)
            plotter.update_gui_signal.emit()  # 发出信号以更新 GUI
            with open(new_path, "a+", encoding='utf-8') as output_file:
                send_ems_ctl(ui, connection, output_file)
                try:
                    data = connection.recv(5000)
                except ConnectionAbortedError as e:
                    print(f"ConnectionAbortedError: {e}")
                    break
                except Exception as e:
                    print(f"An unexpected error occurred: {e}")
                    break

                raw_data_list = [hex(i) for i in data]
                process_data(raw_data_list, connection, output_file)

        # 关闭文件或执行其他操作
        output_file.close()

    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        connection.close()


def run_thread():
    global run_flag
    run_flag = True

    plotter = Plotter()
    plotter.update_gui_signal.connect(lambda: realtime_plot(ui))  # 连接信号到槽

    # 启动原有的 main_function 线程
    thread_fun = threading.Thread(target=main_function, args=(plotter,))
    thread_fun.start()


def close_thread():
    global run_flag
    run_flag = False
    # ui.lineEdit_3.setDisabled(False)
    # ui.doubleSpinBox.setEnabled(True)
    # ui.lineEdit.setDisabled(False)
    print('stopped')


if __name__ == "__main__":
    run_flag = True

    # auto test value
    ems_plan = {}
    autotest_plan = {}
    autotest_flag = 0  # 0 for idle, 1 for test, 2 for judge, 3 for pass, 4 for fail
    autotest_result = 0

    # Q105 Data Init
    batt_workstat = [0, 0, 0, 0]
    soc = [0, 0, 0, 0]
    voltage = [0, 0, 0, 0]
    current = [0, 0, 0, 0]
    acdc_workstat = [0, 0, 0, 0]
    acdc_volt = [0, 0, 0, 0]
    acdc_curr = [0, 0, 0, 0]
    acdc_ext_volt = [0, 0, 0, 0]
    ac_volts = []
    ac_currs = []

    kw_meter_grid = []
    kw_meter_cab = []
    kw_acload = []

    grid_meter_active_pwr = []
    grid_meter_reactive_pwr = []
    grid_meter_acvolts = []
    grid_meter_accurrs = []

    # plot info
    batt_pwr_data = []
    acload_pwr_data = []
    cab_pwr_data = []
    grid_pwr_data = []

    # main thread
    app = QApplication(sys.argv)
    Dialog = QDialog()
    ui = Ui_Dialog()
    ui.setupUi(Dialog)
    Dialog.show()
    ui.pushButton_start.clicked.connect(run_thread)
    ui.pushButton_stop.clicked.connect(close_thread)
    sys.exit(app.exec_())
