# R100
boot_reply = {
    "accept": 1,
    "time": ' '
}

# R101
heart_reply = {
    "time": ' '
}

# R102
r102_reply = {
    "data_from": 0,
    "time": ''
}

# R103
r103_reply = {
    "data_from": 0,
    "time": ''
}

# R104
r104_reply = {
    "data_from": 0,
    "time": ''
}

# R105
r105_reply = {
    "data_from": 0,
    "time": ''
}

# R106
r106_reply = {
    "data_from": 0,
    "time": ''
}

# R107
r107_reply = {
    "time": ''
}

# R108
r108_reply = {
    "data_from": 0,
    "time": ''
}

# R109
r109_reply = {
    "data_from": 0,
    "time": ''
}

# R110
r110_reply = {}

# R111
r111_reply = {
    "accept": 1,
}

# R202
r202_reply = {
    "data_from": 0,
    "time": ''
}

# R204
r204_reply = {
    "data_from": 0,
    "time_stamp": '',
    "time": '',
    "recv_statu": 1
}

# 定义回复信息的映射关系 True/False代表协议内是否发送时间
reply_mapping = {
    'Q100': ('R100', boot_reply, True),
    'Q101': ('R101', heart_reply, True),
    'Q102': ('R102', r102_reply, True),
    'Q103': ('R103', r103_reply, True),
    'Q104': ('R104', r104_reply, True),
    'Q105': ('R105', r105_reply, True),
    'Q106': ('R106', r106_reply, True),
    'Q107': ('R107', r107_reply, True),
    'Q108': ('R108', r108_reply, True),
    'Q109': ('R109', r109_reply, True),
    'Q110': ('R110', r110_reply, False),
    'Q111': ('R111', r111_reply, False),
    'Q202': ('R202', r202_reply, True),
    'Q204': ('R204', r204_reply, True),
}





