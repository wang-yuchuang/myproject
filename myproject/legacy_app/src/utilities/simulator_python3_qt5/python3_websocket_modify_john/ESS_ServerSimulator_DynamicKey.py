import socket
import sys
import time
from threading import Thread
import threading
from PyQt5 import QtCore, QtGui
from PyQt5.QtWidgets import QApplication, QDialog
from ESS_ServerGui import *
import hashlib
import base64
import json
####   #@Author John.Wang

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s
try:
    _encoding = QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QApplication.translate(context, text, disambig)  
      
global SendTime ,run_flag
SendTime = '2017-20-11 14:22:23'
run_flag = True

def Send_WS_packet(packet, connection, f):
    print('Send_WS_packet Packet_Tran:', packet)
    CheckL_list = []

    for i in packet:
        raw = hex(ord(i))
        CheckL_list.append(raw)

    pkt_len = len(CheckL_list)
    opcode = b'\x81'  # 使用字节串表示 opcode

    if pkt_len > 125:
        SecondByte = b'\x7E'
        pkt_len_bytes = pkt_len.to_bytes(2, byteorder='big')
        packet = opcode + SecondByte + pkt_len_bytes + packet.encode('utf-8')
    else:
        pkt_len_byte = pkt_len.to_bytes(1, byteorder='big')
        packet = opcode + pkt_len_byte + packet.encode('utf-8')

    print(packet)
    test = [hex(b) for b in packet]

    connection.sendall(packet)
    CheckL_list = []
    f.write('Packet_Tran:')
    f.write(packet.decode('utf-8', errors='replace'))
    f.write('\n')
    time.sleep(0.5)


"""
def Send_WS_packet(packet,connection,f): #Send CMS packet.req from this function
    print ('Send_WS_packet Packet_Tran : ', packet)
    CheckL_list=[]
    for i in packet:
        raw =hex(ord(i))
        CheckL_list.append(raw)
    pkt_len = len(CheckL_list)
    opcode = chr(129)
    if pkt_len > 125 :
        SecondByte = chr(126) #Changed from FE to 7E i.e. 254 to 126
        pkt_len = hex(pkt_len)
        pkt_len = pkt_len[2:]
        if len(pkt_len) == 3 :
            First = int(pkt_len[0],16)
            First = chr(First)
            Second = int(pkt_len[1:],16)
            Second = chr(Second)
        else:            
            First = chr(0)
            Second = int(pkt_len[0:],16)
            Second = chr(Second)
        packet = opcode + SecondByte + First + Second + packet
    else:   
        #print 'pktlen', pkt_len
        pkt_len = chr(pkt_len)  
        packet = opcode + pkt_len + packet
        
    print(packet)
    test = []
    for i in packet:
        raw =hex(ord(i))
        test.append(raw)
    #print 'Sent pt,', test
    connection.sendall(packet.encode('UTF-8'))
    CheckL_list=[]
    #f.write('Packet_Tran:')
    #f.write(packet)
    #f.write('\n')
    time.sleep(0.5)
"""

"""

def Send_WS_packet_responce(packet,unique_ID,connection,f): #Send CP packet.conf from this function
    packet_1='[3,' + unique_ID + packet
    print( 'Send_WS_packet_responce Packet_Tran:', packet_1)
    CheckL_list=[]
    
#     for i in packet_1:
#         raw =hex(ord(i))
#         CheckL_list.append(raw)
#     pkt_len = len(CheckL_list)
#     pkt_len = chr(pkt_len)
#     opcode = chr(129)
#     packet_1 = opcode + pkt_len + packet_1
    for i in packet_1:
        raw =hex(ord(i))
        CheckL_list.append(raw)
    pkt_len = len(CheckL_list)
    opcode = chr(129)
    if pkt_len > 125 :
        SecondByte = chr(126) #Changed from FE to 7E i.e. 254 to 126
        pkt_len = hex(pkt_len)
        pkt_len = pkt_len[2:]
        if len(pkt_len) == 3 :
            First = int(pkt_len[0],16)
            First = chr(First)
            Second = int(pkt_len[1:],16)
            Second = chr(Second)
        else:            
            First = chr(0)
            Second = int(pkt_len[0:],16)
            Second = chr(Second)
        packet_1 = opcode + SecondByte + First + Second + packet_1
    else:
        pkt_len = chr(pkt_len)
        packet_1 = opcode + pkt_len + packet_1
        test = []
    for i in packet_1:
        #raw =hex(ord(i))
        raw = hex(ord(i))
        test.append(raw)
    #print 'Sent pt,', test

    connection.sendall(packet_1.encode('utf-8'))
    #connection.sendall(packet_1)
    CheckL_list=[]
    f.write('Packet_Tran:')
    f.write(packet_1)
    f.write('\n')
    time.sleep(0.1)
"""

def Send_WS_packet_responce(packet, unique_ID, connection, f, strReplyCmd):
    #packet_1 = f'{{"id": "R100", "{unique_ID}": {packet}}}'
    #packet_1 = f'{{"id": "R100", {unique_ID}, {packet}}}'
    packet_1 = '{{{}, {}, {}}}'.format(strReplyCmd, unique_ID, packet)
    print('Send_WS_packet_responce Packet_Tran:', packet_1)

    pkt_len = len(packet_1)
    opcode = b'\x81'  # 使用字节串表示 opcode

    if pkt_len > 125:
        SecondByte = b'\x7E'
        pkt_len_bytes = pkt_len.to_bytes(2, byteorder='big')
        packet_1 = opcode + SecondByte + pkt_len_bytes + packet_1.encode('utf-8')
    else:
        pkt_len_byte = pkt_len.to_bytes(1, byteorder='big')
        packet_1 = opcode + pkt_len_byte + packet_1.encode('utf-8')

    test = [hex(b) for b in packet_1]  # 使用列表推导简化测试列表的构建

    connection.sendall(packet_1)
    CheckL_list = []

    f.write('Packet_Tran:')
    #f.write(packet_1)  # 将字节串解码为 UTF-8 编码的字符串
    f.write(packet_1.decode('utf-8', errors='replace'))
    #f.write(packet_1.decode('utf-8'))  # 将字节串解码为 UTF-8 编码的字符串并写入文件
    f.write('\n')

    time.sleep(0.1)


def Check_CMS_operations(ui,connection,f):
    global Clear_cache, Data_transfer, RemoteStart, RemoteStop,ReserveNow, CancelReserv
    global ChangeAvailibilty,GetConfig,GetConfig_empty, ChangeConfig,Reset,unlock_connector,Trigger    
    global Firmare_Update, Diagnostics_upload, Send_Local_List ,Send_Local_List_empty
    
    if ui.checkBox.isChecked()==True:                      
        Send_WS_packet(Clear_cache,connection,f)                  
        ui.checkBox.setChecked(False)
    elif ui.checkBox_3.isChecked()==True:                      
        Send_WS_packet(Data_transfer,connection,f)
        ui.checkBox_3.setChecked(False)
    elif ui.checkBox_6.isChecked()==True:                      
        Send_WS_packet(RemoteStart,connection,f) 
        ui.checkBox_6.setChecked(False)
    elif ui.checkBox_7.isChecked()==True:                      
        Send_WS_packet(RemoteStop,connection,f) 
        ui.checkBox_7.setChecked(False)
    elif ui.checkBox_8.isChecked()==True:                      
        Send_WS_packet(ReserveNow,connection,f) 
        ui.checkBox_8.setChecked(False)
    elif ui.checkBox_9.isChecked()==True:                      
        Send_WS_packet(CancelReserv,connection,f) 
        ui.checkBox_9.setChecked(False)
    elif ui.checkBox_10.isChecked()==True:                      
        Send_WS_packet(ChangeAvailibilty,connection,f) 
        ui.checkBox_10.setChecked(False)
    elif ui.checkBox_16.isChecked()==True:                      
        Send_WS_packet(GetConfig,connection,f)
        ui.checkBox_16.setChecked(False)
    elif ui.checkBox_13.isChecked()==True:                      
        Send_WS_packet(GetConfig_empty,connection,f)
        ui.checkBox_13.setChecked(False)
    elif ui.checkBox_17.isChecked()==True:                      
        Send_WS_packet(ChangeConfig,connection,f)
        ui.checkBox_17.setChecked(False)
    elif ui.checkBox_4.isChecked()==True:                      
        Send_WS_packet(Reset,connection,f)
        ui.checkBox_4.setChecked(False)
    elif ui.checkBox_5.isChecked()==True:                      
        Send_WS_packet(unlock_connector,connection,f)
        ui.checkBox_5.setChecked(False)
    elif ui.checkBox_12.isChecked()==True:                      
        Send_WS_packet(Trigger,connection,f) 
        ui.checkBox_12.setChecked(False)
    elif ui.checkBox_18.isChecked()==True:                      
        Send_WS_packet(Firmare_Update,connection,f) 
        ui.checkBox_18.setChecked(False)
    elif ui.checkBox_19.isChecked()==True:                      
        Send_WS_packet(Diagnostics_upload,connection,f) 
        ui.checkBox_19.setChecked(False)
    elif ui.checkBox_20.isChecked()==True:                      
        Send_WS_packet(Send_Local_List,connection,f) 
        ui.checkBox_20.setChecked(False) 
    elif ui.checkBox_21.isChecked()==True:                      
        Send_WS_packet(Send_Local_List_empty,connection,f) 
        ui.checkBox_21.setChecked(False) 

def Update_GUI():    
    global Clear_cache, Data_transfer, RemoteStart, RemoteStop,ReserveNow, CancelReserv, ChangeAvailibilty,GetConfig,GetConfig_empty
    global ChangeConfig,Reset,unlock_connector,Trigger, Diagnostics_reply, Send_Local_List , Send_Local_List_empty
    global Authorize_reply, StartTransaction_reply,StartTransaction_reply_3, StartTransaction_reply_2,StopTransaction_reply 
    global FirmwareStatusNotification_reply, StatusNoti_reply, DataTransfer_reply, MeterValues_reply, Diagnostics_upload, Firmare_Update
    global R103_reply, R104_reply, R105_reply, R106_reply, R107_reply, R108_reply, R109_reply, R110_reply, R202_reply, R204_reply

    StatusNoti_reply = '"content":{"datafrom":0,"time":"'+SendTime+'"}'
    R103_reply = '"content":{"datafrom":0,"time":"'+SendTime+'"}'
    R104_reply = '"content":{"datafrom":0,"time":"'+SendTime+'"}'
    R105_reply = '"content":{"datafrom":0,"time":"'+SendTime+'"}'
    R106_reply = '"content":{"datafrom":0,"time":"'+SendTime+'"}'
    R107_reply = '"content":{"datafrom":0,"time":"'+SendTime+'"}'
    R108_reply = '"content":{"datafrom":0,"time":"'+SendTime+'"}'
    R109_reply = '"content":{"datafrom":0,"time":"'+SendTime+'"}'
    R110_reply = '"content":{"datafrom":0,"time":"'+SendTime+'"}'
    R202_reply = '"content":{"datafrom":0,"time":"'+SendTime+'"}'
    R204_reply = '"content":{"datafrom":0,"time":"'+SendTime+'"}'
    DataTransfer_reply = ',{"data":"dummy","status":"Accepted"}'
    MeterValues_reply = ',{}]'
    #Authorize_reply  = ',{"idTagInfo":{"status":"'+str(ui.comboBox_15.currentText())+'","expiryDate":"2019-05-16T15:02:01Z","parentIdTag":"NULL"}}]'
    #Authorize_reply  = ',{"idTagInfo":{"status":"'+str(ui.comboBox_15.currentText())+'"}}]'
    Authorize_reply  = '"content":{"datafrom":0,"time":"'+SendTime+'"}'
    StartTransaction_reply = ',{"idTagInfo":{"status":"'+str(ui.comboBox_14.currentText())+'"},"transactionId":2822478}]'
    StartTransaction_reply_2 = ',{"idTagInfo":{"status":"'+str(ui.comboBox_14.currentText())+'"},"transactionId":282278}]'
    StartTransaction_reply_3 = ',{"idTagInfo":{"status":"'+str(ui.comboBox_14.currentText())+'"},"transactionId":2822479}]'
    StopTransaction_reply = ',{"idTagInfo":{"status":"'+str(ui.comboBox_16.currentText())+'"}}]'
    FirmwareStatusNotification_reply = ',{}]' 
    Diagnostics_reply =',{}]'
    Clear_cache = '[2,"980","ClearCache",{}]' 
    Data_transfer = '[2,"984","DataTransfer",{"vendorId":"CPV07","messageId":"demo","data":"sample"}]' 
    
    if ui.comboBox_11.currentIndex() == 0:
        Remote_id_tag = '"5169F25656"'        
    elif ui.comboBox_11.currentIndex() == 1:
        Remote_id_tag = '"1509647633608"'
    else:
        Remote_id_tag = '"516964"' 
        
    Stop_tran = '1509647633606'  
    RemoteStart = '[2,"987","RemoteStartTransaction",{"connectorId":'+str(ui.comboBox_11.currentText())+',"idTag":'+Remote_id_tag+'}]'
    if ui.comboBox_12.currentIndex() == 0:
        Stop_tran = '1509647633606'      
    elif ui.comboBox_12.currentIndex() == 1:
        Stop_tran = '1509647633608'       
    else:
        Stop_tran = '1509647633609'  
     
    RemoteStop = '[2,"990","RemoteStopTransaction",{"transactionId":'+Stop_tran+'}]'
 
    if ui.comboBox_9.currentIndex() == 0:
        Reserve_id_tag = '"211535"' 
        reservationId = '1509404930281'       
    elif ui.comboBox_9.currentIndex() == 1:
        Reserve_id_tag = '"211536"'  
        reservationId = '1509404930282'         
    else:
        Reserve_id_tag = '"211537"'    
        reservationId = '1509404930283'          
    if ui.comboBox_10.currentIndex() == 0:
        cancelreservationId = '1509404930281'       
    elif ui.comboBox_10.currentIndex() == 1:
        cancelreservationId = '1509404930282'         
    else:
        cancelreservationId = '1509404930283'  
        
    ReserveNow = '[2,"992","ReserveNow",{"connectorId":'+str(ui.comboBox_9.currentText())+',"idTag":'+Reserve_id_tag+',"parentIdTag":"0","reservationId":'+reservationId+',"expiryDate":"2019-11-02T16:41:37Z"}]'
    CancelReserv = '[2,"993","CancelReservation",{"reservationId":'+cancelreservationId+'}]'
    
    ChangeAvailibilty= '[2,"998","ChangeAvailability",{"connectorId":'+str(ui.comboBox_6.currentText())+',"type":"'+str(ui.comboBox_5.currentText())+'"}]'
    
    GetConfig = '[2,"974","GetConfiguration",{"key":["'+str(ui.comboBox.currentText())+'"]}]'
    GetConfig_empty = '[2,"974","GetConfiguration",{"key":[]}]'
    #list = ui.lineEdit_2.text
    #list1 = list[1:0]
    #print list1,list
    #if(str(ui.lineEdit_2.text[0].find) == '['):
    ChangeConfig= '[2,"979","ChangeConfiguration",{"key":"'+str(ui.comboBox_2.currentText())+'","value":"'+str(ui.lineEdit_2.text())+'"}]'
    #else:
       # ChangeConfig= '[2,"979","ChangeConfiguration",{"key":"'+str(ui.comboBox_2.currentText())+'","value":'+str(ui.lineEdit_2.text())+'}]'
    
    Reset = '[2,"985","Reset",{"type":"'+str(ui.comboBox_7.currentText())+'"}]'
    
    unlock_connector = '[2,"981","UnlockConnector",{"connectorId":'+str(ui.comboBox_13.currentText())+'}]'
    
    Trigger = '[2,"975","TriggerMessage",{"requestedMessage":"'+str(ui.comboBox_3.currentText())+'","connectorId":'+str(ui.comboBox_4.currentText())+'}]'     
    
    Send_Local_List = '[2,"33","SendLocalList",{"listVersion":0,"localAuthorizationList":[{"idTag":"899999","idTagInfo":{"status":"Accepted","expiryDate":"2019-11-02T16:41:37Z","parentIdTag":"NULL"}},{"idTag":"799999","idTagInfo":{"status":"Accepted","expiryDate":"2019-11-02T16:41:37Z","parentIdTag":"NULL"}}],"updateType":"Full"}]'
    Send_Local_List_empty = '[2,"33","SendLocalList",{"listVersion":0,"localAuthorizationList":[{}],"updateType":"Full"}]'
    
    Diagnostics_upload = '[2,"432","GetDiagnostics",{"location":"'+str(ui.lineEdit_7.text())+'/","retries":3,"retryInterval":2,"startTime":"'+str(ui.lineEdit_4.text())+'","stopTime":"'+str(ui.lineEdit_5.text())+'"}]'
    Firmare_Update = '[2,"232","UpdateFirmware",{"location":"'+str(ui.lineEdit_7.text())+'/'+str(ui.lineEdit_8.text())+'","retries":3,"retrieveDate":"'+str(ui.lineEdit_4.text())+'","retryInterval":2}]'
    #ftp://validation:Val_123@172.16.65.130:21/EV_Charger
    
def Reply_for_CP_request(list_d,Check_for_reply,connection,f,unique_ID):
    list_d=list_d
    print('request: Check_for_reply', Check_for_reply)
    strReplyCmd = '"id": "R100"';

    if Check_for_reply=='{"id":"Q100"':
        #Send_WS_packet(GetConfig_empty,connection,f)
        #time.sleep(0.5)
        Send_WS_packet_responce(boot_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]
    elif Check_for_reply=='{"id":"Q101"':
        strReplyCmd = '"id": "R101"'
        Send_WS_packet_responce(heart_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]                                
    elif Check_for_reply=='{"id":"Q105"':
        strReplyCmd = '"id": "R105"'
        Send_WS_packet_responce(StatusNoti_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]                                
    elif Check_for_reply=='"DataTransfer"':
        Send_WS_packet_responce(DataTransfer_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]                                
    elif Check_for_reply=='{"id":"Q102"':
        strReplyCmd = '"id": "R102"';
        Send_WS_packet_responce(Authorize_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]
    elif Check_for_reply=='{"id":"Q103"':
        strReplyCmd = '"id": "R103"';
        Send_WS_packet_responce(R103_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]
    elif Check_for_reply=='{"id":"Q104"':
        strReplyCmd = '"id": "R104"';
        Send_WS_packet_responce(R104_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]
    elif Check_for_reply=='{"id":"Q105"':
        strReplyCmd = '"id": "R105"';
        Send_WS_packet_responce(R105_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]
    elif Check_for_reply=='{"id":"Q106"':
        strReplyCmd = '"id": "R106"';
        Send_WS_packet_responce(R106_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]
    elif Check_for_reply=='{"id":"Q107"':
        strReplyCmd = '"id": "R107"';
        Send_WS_packet_responce(R107_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]
    elif Check_for_reply=='{"id":"Q108"':
        strReplyCmd = '"id": "R108"';
        Send_WS_packet_responce(R108_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]
    elif Check_for_reply=='{"id":"Q109"':
        strReplyCmd = '"id": "R109"';
        Send_WS_packet_responce(R109_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]
    elif Check_for_reply=='{"id":"Q202"':
        strReplyCmd = '"id": "R202"';
        Send_WS_packet_responce(R202_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]
    elif Check_for_reply=='{"id":"Q204"':
        strReplyCmd = '"id": "R204"';
        Send_WS_packet_responce(R204_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]
    elif Check_for_reply=='"StartTransaction"' : 
        if list_d[3]== '{"connectorId":1':
            Send_WS_packet_responce(StartTransaction_reply,unique_ID,connection,f,strReplyCmd)
        elif list_d[3]== '{"connectorId":2': 
            Send_WS_packet_responce(StartTransaction_reply_2,unique_ID,connection,f,strReplyCmd)
        elif list_d[3]== '{"connectorId":3': 
            Send_WS_packet_responce(StartTransaction_reply_3,unique_ID,connection,f,strReplyCmd)
        list_d=[]                                
    elif Check_for_reply=='"MeterValues"':
        Send_WS_packet_responce(MeterValues_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]                                
    elif Check_for_reply=='"StopTransaction"':
        Send_WS_packet_responce(StopTransaction_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]    
    elif Check_for_reply=='"FirmwareStatusNotification"':
        Send_WS_packet_responce(FirmwareStatusNotification_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]   
    elif Check_for_reply=='"DiagnosticsStatusNotification"':
        Send_WS_packet_responce(Diagnostics_reply,unique_ID,connection,f,strReplyCmd)
        list_d=[]                                
    else:
        f.close
        list_d=[]          
    return list_d
def CreateWSkey(CPkey):
    WS_key = '258EAFA5-E914-47DA-95CA-C5AB0DC85B11'
    SHA_1 = CPkey + WS_key    
    #SHA_1 = hashlib.sha1(SHA_1).digest()
    SHA_1 = hashlib.sha1(SHA_1.encode()).digest()
    SHA_1_base64 = base64.b64encode(SHA_1)    
    #print 'SHA_1  :', SHA_1    
    #print 'Base64 :', SHA_1_base64    
    return SHA_1_base64

def Main_function():
    global Clear_cache, Data_transfer, RemoteStart, RemoteStop,ReserveNow, CancelReserv, ChangeAvailibilty,GetConfig,GetConfig_empty        
    global Authorize_reply, StartTransaction_reply, StopTransaction_reply , FirmwareStatusNotification_reply,StartTransaction_reply_3
    global StartTransaction_reply_2,StartTransaction_reply_3, ChangeConfig,Reset,unlock_connector,Trigger 
    global StatusNoti_reply, DataTransfer_reply, MeterValues_reply
    global SendTime, Log_file_SendTime 
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    ServerIP = ui.lineEdit_3.text()
    serverPort = int(float(ui.doubleSpinBox.text()))
    server_address = (ServerIP, serverPort) #192.168.162.210
    print (server_address)
    ui.lineEdit_3.setDisabled(True)
    ui.doubleSpinBox.setEnabled(False)
    ui.lineEdit.setDisabled(True)
    #print >>sys.stderr, 'starting up on %s port %s' % server_address
    print('starting up on %s port %s' % server_address, file=sys.stderr)
    sock.bind(server_address)
    sock.listen(1) #Argument 1 means it can handle the one client, if 2 it can handle two client a a time. 
    print ('Waiting for a connection...')
    connection , client_address = sock.accept()
    print ('TCP :',client_address )
           
    list_d=[]      
    try:
        #print >>sys.stderr, 'Connection from', client_address
        print('Connection from', client_address, file=sys.stderr)
        Get_time_now = time.localtime()
        Log_file_SendTime=  '_'+str(Get_time_now[0])+str(Get_time_now[1])+str(Get_time_now[2])+'_'+str(Get_time_now[3])+str(Get_time_now[4])+str(Get_time_now[5])
        File_name= ui.lineEdit.text()+Log_file_SendTime+'.txt'       
                     
        while run_flag == True:           
            global Clear_cache, Data_transfer, RemoteStart, RemoteStop,ReserveNow, CancelReserv, ChangeAvailibilty,GetConfig,GetConfig_empty
            global ChangeConfig,Reset,unlock_connector,Trigger, Diagnostics_upload ,Diagnostics_reply  
            global Authorize_reply, StartTransaction_reply, StopTransaction_reply ,Firmare_Update ,FirmwareStatusNotification_reply            
            global boot_reply, heart_reply, Send_Local_List ,Send_Local_List_empty
            
            Update_GUI() #This will update CMS packet from GUI
            #boot_reply = ',{{"status":"{}","currentTime":"{}","interval":{}}}]'.format(ui.comboBox_8.currentText(), SendTime, ui.lineEdit_9.text())
            boot_reply = '"content":{"accept":'+str(ui.comboBox_8.currentIndex())+',"time":"'+SendTime+'"'+'}'
            heart_reply = '"content":{"time":"'+SendTime+'"}'
            
            Get_time_now = time.localtime()
            SendTime=  str(Get_time_now[0])+'-'+str(Get_time_now[1])+'-'+str(Get_time_now[2])+'T'+str(Get_time_now[3])+':'+str(Get_time_now[4])+':'+str(Get_time_now[5])+'Z'
            f= open(File_name,"a+", encoding='utf-8')
            Check_CMS_operations(ui,connection,f)             
            data = []
            data = connection.recv(3000)
            #print('Rawdata_Recv : ',data)
            Raw_data_list=[]
            
            for i in data:
                #raw =hex(ord(i))
                raw = hex(ord(chr(i)))
                Raw_data_list.append(raw)
            #print 'Raw Data Received   : ', Raw_data_list
            #print 'recdPpkt byte lenth :', len(Raw_data_list)
            if Raw_data_list !=[]:
                None
            while Raw_data_list != [] :
                RawDATA = Raw_data_list #raw data for print use only
                
                if Raw_data_list[0]=='0x47' and Raw_data_list[1]=='0x45' and Raw_data_list[2]=='0x54':
                    print (data)
                    Handshake_list = []
                    for i in Raw_data_list:
                        hand_1 = chr(int(i,16)) 
                        Handshake_list.append(hand_1)
                    Handshake_list = ''.join(map(str, Handshake_list))
                    #print Handshake_list
                    Handshake_list = Handshake_list.split('\r\n')   
                      
                    #print 'Handshake_list', Handshake_list 
                    Rcvd_key =  Handshake_list[6]
                    Rcvd_key = Rcvd_key.split(':') #['Sec-WebSocket-Key', '9iPJw3of+czlqg1MbLLnrA==']
                    Rcvd_key = Rcvd_key[1]
                    Rcvd_key = Rcvd_key[1:]
                    #print 'Rcvd_key', Rcvd_key
                    
                    WebsocketKey = CreateWSkey(Rcvd_key)
                    
                    handshake = ('HTTP/1.1 101 Switching Protocols' '\r\n'
                                 'Upgrade: websocket' '\r\n'
                                 'Connection: Upgrade' '\r\n' 
                                 'Sec-WebSocket-Accept: ' + str(WebsocketKey) + '\r\n' 
                                 'Sec-WebSocket-Protocol: ESS Websocket' '\r\n' '\r\n') #'Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=' '\n'


                    #connection.sendall(handshake)
                    connection.sendall(handshake.encode())
                    #print data
                    print( handshake)
                    f.write('Handshake Response sent')
                    f.write('\n') 
                    time.sleep(2)
                    Raw_data_list=[]                        
                else:
                    #print('org_data:', Raw_data_list)
                    mask_code=[]
                    if (Raw_data_list[0]=='0x1' or Raw_data_list[0]=='0x80' or Raw_data_list[0]=='0x81') and Raw_data_list[1] != '0xfe':
                        max_data = int((Raw_data_list[1]),16) - 128
                        Required_convrt_data = Raw_data_list[6:]
                        remove_data = Raw_data_list[:6]
                        mask_code_1 = remove_data[2:]
                        for i in mask_code_1:
                                i = chr(int(i,16))
                                mask_code.append(i)
                        #print 'Mask_key_recived_normal_pckt',mask_code 
                        for i in remove_data:
                            Raw_data_list.remove(i)
                        Required_convrt_data = Required_convrt_data[:max_data]
                        
                    elif Raw_data_list[1]=='0xfe':
                        Secondbyte = Raw_data_list[3]
                        Secondbyte = Secondbyte[2:]
                        if len(Secondbyte)==1:
                            Secondbyte = "%s%s" % ('0',Secondbyte)
                        MaxData_1 ="%s%s" % (Raw_data_list[2],Secondbyte)
                        max_data = int(MaxData_1,16)
                        #print 'max_data', max_data 
                        Required_convrt_data = Raw_data_list[8:]
                        remove_data = Raw_data_list[:8]
                        mask_code_1 = remove_data[4:]
                        for i in mask_code_1:
                                i = chr(int(i,16))
                                mask_code.append(i)
                        #print 'Mask_key_recived_ext_pckt',mask_code 
                        for i in remove_data:
                            Raw_data_list.remove(i)
                        Required_convrt_data = Required_convrt_data[:max_data]
                        #print Required_convrt_data
                        #print len(Required_convrt_data)
                    elif Raw_data_list[0]=='0x89' and Raw_data_list[1]=='0x80':
                        print('Recv Ping Cmd',RawDATA)
                        break
                    else:
                        print ('Wrong packet')
                        print('Raw_data_list -',RawDATA)
                        break
                    index = 0
                    print (Required_convrt_data)
                    for i in Required_convrt_data: 
                        test = int(i,16) ^ ord(mask_code[index])
                        test= chr(test)
                        if test == '\n' or test=='\t':                            
                            None
                        else:
                            list_d.append(test)
                        
                        index = index+1
                        if index == 4:
                            index= 0                        
                    try :
                        for i in Required_convrt_data:
                            Raw_data_list.remove(i)  
                    except Exception as err:
                        print (err )
                        #pass               
                    print ("list_d: "+str(list_d))
                    print('Required_convrt_data  ', Required_convrt_data)
                    print('max_data', max_data)
                    print('list_d[-1]', list_d[-1])
                    print('len(Required_convrt_data)', len(Required_convrt_data))
                    if len(Required_convrt_data) >= max_data and list_d[-1]=='}' :
                        list_d = ''.join(map(str, list_d))
                        test_flag = list_d
                        list_d = list_d.split(',')              
                        print( 'Packet_Recv : ',test_flag)
                        f.write('Packet_Recv : ')
                        f.write(str(list_d))
                        f.write('\n')

                        original_string = ''
                        #for item in list_d:
                        #    original_string += item
                        original_string = json.dumps(list_d)
                        parsed_data = json.loads(original_string)
                        formatted_string = json.dumps(parsed_data, indent=2)
                        print('formatted_string', formatted_string)
                        # Access the values of the keys
                        if isinstance(parsed_data, list):
                            # If the parsed_data is a list, access its elements by index
                            id_value = parsed_data[0]  # Assuming the "id" value is the first element in the list
                        else:
                            # If the parsed_data is a dictionary, access its values as usual
                            id_value = parsed_data["id"]

                        print("id:", id_value)
                        if len(list_d) >1:              
                            #unique_ID = list_d[1]
                            #Check_for_reply = list_d[2]
                            Check_for_reply = list_d[0]
                            unique_ID = list_d[1]
                            print('list_d[0][1][2][3]', list_d[0], list_d[1], list_d[2], list_d[3])
                            #original_string = list_d[0] + list_d[1] + list_d[2] + list_d[3]
                            #parsed_data = json.loads(original_string)
                            #formatted_string = json.dumps(parsed_data, indent=2)
                            #print(formatted_string)
                            print('Check_for_reply', Check_for_reply)
                            list_d = Reply_for_CP_request(list_d,Check_for_reply,connection,f,unique_ID)
                            list_d = []
                        else:
                            list_d=[] 
                            None
                    else:
                        None
    finally:
        connection.close()  
                  
def Main_run():
    thread_fun=threading.Thread(target=Main_function)
    thread_fun.start()    
          
def run_thread():
    global run_flag
    run_flag=True
    Main_run()    
    
def close_thread():
    global run_flag
    run_flag=False
    ui.lineEdit_3.setDisabled(False)
    ui.doubleSpinBox.setEnabled(True)
    ui.lineEdit.setDisabled(False)
    print('stopped')
      
if __name__ == "__main__":
    import sys
    app = QApplication(sys.argv)
    Dialog = QDialog()
    ui = Ui_Dialog()
    ui.setupUi(Dialog)
    Dialog.show()
    #QtCore.QObject.connect(ui.pushButton, QtCore.SIGNAL(_fromUtf8("clicked()")), run_thread)
    #QtCore.QObject.connect(ui.pushButton_2, QtCore.SIGNAL(_fromUtf8("clicked()")), close_thread)
    ui.pushButton.clicked.connect(run_thread)
    ui.pushButton_2.clicked.connect(close_thread)
    sys.exit(app.exec_())
    
    
