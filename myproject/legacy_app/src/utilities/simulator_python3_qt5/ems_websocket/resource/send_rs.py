# 401
q401 = {
    "id": "Q401",
    "uniqKey": "101010401",
    "content": {
        "rst_type": 0
    }
}

# 402
q402 = {
    "id": "Q402",
    "uniqKey": "101010402",
    "content": {
        "cmd_type": 0
    }
}

# 404
q404 = {
    "id": "Q404",
    "uniqKey": "101010404",
    "content": {
        "url": 'http://127.0.0.1:',
        'sw_pkg_ver': 0,
    }
}

# 406
q406 = {
    "id": "Q406",
    "uniqKey": "101010406",
    "content": {
        "url": 'http://127.0.0.1:',
        "diag_type": 1,
        "start_time": '',
        "stop_time": ''
    }
}

# 411
q411 = {
    "id": "Q411",
    "uniqKey": "101010411",
    "content": {
        "keys": ''
    }
}

# 412
q412 = {
    "id": "Q412",
    "uniqKey": "101010412",
    "content": {
        "key_values": [
            # {"val": "0", "name": "Protect_SOC"}
            # {"val": "150.00", "name": "SysACInput_PwrLmt"},
            # {"val": "50.00", "name": "Max_Output2AC_PwrLmt"},
            # {"val": "20.00", "name": "Min_Output2AC_PwrLmt"},
            # {"val": "0.00", "name": "Max_Distri2DCDC_PwrLmt"},
            # {"val": "1.00", "name": "Output2AC_PwrRate"},
            # {"val": "0.00", "name": "Min_ACLoad_StartDisChg"},
            # {"val": "500.00", "name": "UrgetAlterDischrgCurr"},
            # {"val": "50.00", "name": "AdjustUrgetModeCurr"},
            # {"val": "9999.00", "name": "EnterUrgetDisChrgModePower"},
            # {"val": "40.00", "name": "EixtUrgetDischrgModePower"},
            # {"val": "10", "name": "Cmd_101_Interval"},
            # {"val": "10", "name": "Cmd_102_Interval"},
            # {"val": "60", "name": "Cmd_103_Interval"},
            # {"val": "600", "name": "Cmd_104_Interval"},
            # {"val": "20", "name": "Cmd_105_Interval"},
            # {"val": "600", "name": "Cmd_106_Interval"},
            {"val": "30", "name": "Cmd_107_Interval"},
            # {"val": "600", "name": "Cmd_108_Interval"},
            # {"val": "600", "name": "Cmd_109_Interval"},
            # {"val": "10", "name": "Cmd_110_Interval"},
            # {"val": "300", "name": "Cmd_204_Interval"},
            # {"val": "180", "name": "Cmd_202_Interval"}
        ]
    }
}


# 503
read_ems_plan = {
    "id": "Q503",
    "uniqKey": "101010503",
    "content": {}
}

# 504
default_ems_plan = {
    "id": "Q504",
    "uniqKey": "101010504",
    "content": {
        "ems_plan_ver": 51,
        "ems_plan_cfg": [
            {
                "reactv_pwr_lmt_rate": 50,
                "actv_pwr_lmt_rate": 50,
                "batt_workmode": 1,
                "start_time": "1970-01-01 00:00:00",
                "stop_soc": 90,
                "battgrp_pwr_lmt_rate": 20,
                "interval": 39600
            },
            {
                "reactv_pwr_lmt_rate": 50,
                "actv_pwr_lmt_rate": 50,
                "batt_workmode": 1,
                "start_time": "1970-01-01 11:00:00",
                "stop_soc": 100,
                "battgrp_pwr_lmt_rate": 20,
                "interval": 21000
            },
            {
                "reactv_pwr_lmt_rate": 50,
                "actv_pwr_lmt_rate": 50,
                "batt_workmode": 0,
                "start_time": "1970-01-01 16:50:00",
                "stop_soc": 50,
                "battgrp_pwr_lmt_rate": 20,
                "interval": 12900
            },
            {
                "reactv_pwr_lmt_rate": 50,
                "actv_pwr_lmt_rate": 50,
                "batt_workmode": 1,
                "start_time": "1970-01-01 20:25:00",
                "stop_soc": 50,
                "battgrp_pwr_lmt_rate": 20,
                "interval": 300
            },
            {
                "reactv_pwr_lmt_rate": 50,
                "actv_pwr_lmt_rate": 50,
                "batt_workmode": 0,
                "start_time": "1970-01-01 20:30:00",
                "stop_soc": 50,
                "battgrp_pwr_lmt_rate": 20,
                "interval": 300
            },
            {
                "reactv_pwr_lmt_rate": 50,
                "actv_pwr_lmt_rate": 50,
                "batt_workmode": 1,
                "start_time": "1970-01-01 20:35:00",
                "stop_soc": 50,
                "battgrp_pwr_lmt_rate": 20,
                "interval": 8700
            },
            {
                "reactv_pwr_lmt_rate": 50,
                "actv_pwr_lmt_rate": 50,
                "batt_workmode": 1,
                "start_time": "1970-01-01 23:00:00",
                "stop_soc": 50,
                "battgrp_pwr_lmt_rate": 20,
                "interval": 3600
            }
        ],
        "ems_plan_repeat": 1
    }
}

# 601
q601 = {
    "id": "Q601",
    "uniqKey": "101010601",
    "content": {}
}

# 602
q602 = {
    "id": "Q602",
    "uniqKey": "101010602",
    "content": {
        "fee_rate_cfg": ''
    }
}

# auto test
TIME_GAP = 5  # Minute
# workmode = [1, 0]  # chg->dischg
workmode = [0, 1]  # dischg->chg
autotest_ems_plan = {
    "id": "Q504",
    "uniqKey": "101010999",
    "content": {
        "ems_plan_ver": 0,
        "ems_plan_cfg": [
            {
                "reactv_pwr_lmt_rate": 50,
                "actv_pwr_lmt_rate": 50,
                "batt_workmode": workmode[0],
                "start_time": "1970-01-01 00:00:00",
                "stop_soc": 10,
                "battgrp_pwr_lmt_rate": 20,
                "interval": TIME_GAP * 60  # Min * 60
            },
            {
                "reactv_pwr_lmt_rate": 50,
                "actv_pwr_lmt_rate": 50,
                "batt_workmode": workmode[1],
                "start_time": "1970-01-01 11:00:00",
                "stop_soc": 100,
                "battgrp_pwr_lmt_rate": 20,
                "interval": TIME_GAP * 60  # Min * 60
            }
        ],
        "ems_plan_repeat": 1
    }
}


