import tkinter as tk
from tkinter import ttk


def update_and_return(old_ems_plan):
    global entry_widgets
    global new_ems_plan
    for i, config in enumerate(old_ems_plan["content"]["ems_plan_cfg"]):
        for key, entry in entry_widgets[i].items():
            try:
                config[key] = float(entry.get())
            except ValueError:
                config[key] = entry.get()
    new_ems_plan = old_ems_plan.copy()


def add_row(old_ems_plan, index):
    new_config = {key: 0 for key in old_ems_plan["content"]["ems_plan_cfg"][0].keys()}
    old_ems_plan["content"]["ems_plan_cfg"].insert(index + 1, new_config)
    render_ems_plan_gui(old_ems_plan)


def delete_row(old_ems_plan, index):
    if len(old_ems_plan["content"]["ems_plan_cfg"]) > 1:
        old_ems_plan["content"]["ems_plan_cfg"].pop(index)
        render_ems_plan_gui(old_ems_plan)


def render_ems_plan_gui(old_ems_plan):
    global entry_widgets

    for widget in root.winfo_children():
        widget.destroy()

    button_frame = ttk.Frame(root)
    button_frame.grid(row=0, column=0, columnspan=3, pady=10, padx=5)

    ttk.Label(root, text="EMS Plan Configuration").grid(row=1, column=0, columnspan=3, pady=10)

    for j, key in enumerate(old_ems_plan["content"]["ems_plan_cfg"][0].keys()):
        ttk.Label(root, text=key).grid(row=2, column=2 * j, sticky="w", padx=5, pady=2)

    entry_widgets = {}
    for j, config in enumerate(old_ems_plan["content"]["ems_plan_cfg"]):
        entry_widgets[j] = {}
        for i, (key, value) in enumerate(config.items()):
            ttk.Entry(root).grid(row=j + 3, column=2 * i, sticky="w", padx=5, pady=2)
            entry_widgets[j][key] = root.grid_slaves(row=j + 3, column=2 * i)[0]
            entry_widgets[j][key].insert(0, str(value))

        ttk.Button(root, text="+", command=lambda j=j: add_row(old_ems_plan, j)).grid(row=j + 3, column=len(config) * 2,
                                                                                      padx=5)
        ttk.Button(root, text="-", command=lambda j=j: delete_row(old_ems_plan, j)).grid(row=j + 3,
                                                                                         column=len(config) * 2 + 1,
                                                                                         padx=5)

    update_button = ttk.Button(button_frame, text="Update Values", command=lambda: update_and_return(old_ems_plan))
    update_button.grid(row=0, column=0, pady=0, padx=5, sticky="w")


def get_ems_plan_gui(old_ems_plan):
    global root
    global entry_widgets

    root = tk.Tk()
    root.title("EMS Plan UI")
    render_ems_plan_gui(old_ems_plan)
    root.mainloop()

    return new_ems_plan


# 示例使用
if __name__ == "__main__":
    old_plan = {
    "id": "Q504",
    "uniqKey": "101010504",
    "content": {
        "ems_plan_ver": 51,
        "ems_plan_cfg": [
            {
                "reactv_pwr_lmt_rate": 100,
                "actv_pwr_lmt_rate": 375,
                "batt_workmode": 1,
                "start_time": "1970-01-01 00:00:00",
                "stop_soc": 90,
                "battgrp_pwr_lmt_rate": 110,
                "interval": 39600
            },
            {
                "reactv_pwr_lmt_rate": 100,
                "actv_pwr_lmt_rate": 375,
                "batt_workmode": 1,
                "start_time": "1970-01-01 11:00:00",
                "stop_soc": 100,
                "battgrp_pwr_lmt_rate": 110,
                "interval": 21000
            },
            {
                "reactv_pwr_lmt_rate": 100,
                "actv_pwr_lmt_rate": 375,
                "batt_workmode": 0,
                "start_time": "1970-01-01 16:50:00",
                "stop_soc": 50,
                "battgrp_pwr_lmt_rate": 110,
                "interval": 12900
            },
            {
                "reactv_pwr_lmt_rate": 100,
                "actv_pwr_lmt_rate": 375,
                "batt_workmode": 1,
                "start_time": "1970-01-01 20:25:00",
                "stop_soc": 50,
                "battgrp_pwr_lmt_rate": 110,
                "interval": 300
            },
            {
                "reactv_pwr_lmt_rate": 100,
                "actv_pwr_lmt_rate": 375,
                "batt_workmode": 0,
                "start_time": "1970-01-01 20:30:00",
                "stop_soc": 50,
                "battgrp_pwr_lmt_rate": 110,
                "interval": 300
            },
            {
                "reactv_pwr_lmt_rate": 100,
                "actv_pwr_lmt_rate": 375,
                "batt_workmode": 1,
                "start_time": "1970-01-01 20:35:00",
                "stop_soc": 50,
                "battgrp_pwr_lmt_rate": 110,
                "interval": 8700
            },
            {
                "reactv_pwr_lmt_rate": 100,
                "actv_pwr_lmt_rate": 375,
                "batt_workmode": 1,
                "start_time": "1970-01-01 23:00:00",
                "stop_soc": 50,
                "battgrp_pwr_lmt_rate": 110,
                "interval": 3600
            }
        ],
        "ems_plan_repeat": 1
    }
}
    new = get_ems_plan_gui(old_plan)
    print("Updated EMS Plan:", new)
