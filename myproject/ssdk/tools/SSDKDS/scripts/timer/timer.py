from file_ops import FileBase
from excel_ops import PyxlExcelReader
import project

# for timer_cfg.c file


class TimerSrcFile(FileBase):
    body_template = \
"""
#ifdef CONFIG_TIMER

#include "timer_cfg.h"

#include <types.h>
#include <board.h>

#include <driver/timer/arch_timer.h>

/* timer device driver headers */
%s
/* timer device drivers */
%s
/* timer config structures */
%s
/* timer initialization */
%s

#endif
"""

    config_struct_template_sdrv = \
"""static const sdrv_tmr_cfg_t %s = {
    .base = %s,
    .irq = %s,
    .clk_sel = %s,
    .clk_frq = %s,
    .clk_div = %s,
};
"""

    config_struct_template_xli = \
"""static const xli_ttc_cfg_t %s = {
    .base = %s,
    .irq = %s,
    .clk_frq = %s,
};
"""

    config_struct_template_btm = \
"""static const sdrv_btm_cfg_t %s = {
    .base = %s,
    .irq = %s,
    .tmr_id = SDRV_BTM_G1,
    .tmr_cfg = {
        .si_val = 23,
        .inc_val = 1,
        .frc_rld_rst_cnt_en = true,
        .term_use_mode = SDRV_BTM_SHADOW,
        .cmp_use_mode = SDRV_BTM_SHADOW,
        .cnt_dir = SDRV_BTM_CNT_UP,
        .cnt_mode = SDRV_BTM_CONTINOUS_MODE,
    }
};
"""

    timer_init_template = \
"""void board_timer_init(void)
{
%s}
"""

    def _driver_headers_gen(self, IP_name):
        driver_headers = ''
        if IP_name == 'SemiDrive':
            driver_headers += '#include <driver/timer/sdrv_tmr_drv.h>\n'
        elif IP_name == 'Xilinx':
            driver_headers += '#include <driver/timer/xli_ttc_drv.h>\n'
        elif IP_name == 'BTM':
            driver_headers += '#include <driver/timer/sdrv_btm_drv.h>\n'
        return driver_headers

    def _device_drivers_gen(self, data_dict, IP_name):
        device_drivers = ''
        if IP_name == 'SemiDrive':
            for i in range(0, len(data_dict['Name'])):
                name = data_dict['Name'][i]
                dev_name = 'g_sdrv_%s' % name
                lowdev_name = 'g_%s' % name

                device_drivers += \
                    'sdrv_tmr_dev_t %s;\n' % dev_name
                device_drivers += \
                    'timer_dev_t *%s;\n\n' % lowdev_name

        elif IP_name == 'Xilinx':
            for i in range(0, len(data_dict['Name'])):
                name = data_dict['Name'][i]
                dev_name = 'g_xli_%s' % name
                lowdev_name = 'g_%s' % name

                device_drivers += \
                    'xli_ttc_dev_t %s;\n' % dev_name
                device_drivers += \
                    'timer_dev_t *%s;\n\n' % lowdev_name

        elif IP_name == 'BTM':
            for i in range(0, len(data_dict['Name'])):
                name = data_dict['Name'][i]
                dev_name = 'g_sdrv_btm_%s' % name
                lowdev_name = 'g_%s' % name

                device_drivers += \
                    'sdrv_btm_dev_t %s;\n' % dev_name
                device_drivers += \
                    'timer_dev_t *%s;\n\n' % lowdev_name
        return device_drivers

    def _config_structs_gen(self, data_dict, IP_name):
        config_structs = ''
        if IP_name == 'SemiDrive':
            for i in range(0, len(data_dict['Name'])):
                name = str(data_dict['Name'][i])
                timer_no = str(data_dict['Timer NO.'][i])
                irq = str(data_dict['IRQ'][i])
                clk_sel = str(data_dict['Clock Source'][i])
                clk_frq = str(data_dict['Clock Frequency'][i])
                clk_div = str(data_dict['Clock Division'][i])

                config_structs += TimerSrcFile.config_struct_template_sdrv % (
                    '%s_config' % name,
                    'DEVICE_BASE(%s)' % timer_no,
                    ('DEVICE_INTR(%s)' %
                     (timer_no + '_CHN')) if irq == 'On' else '-1',
                    clk_sel,
                    clk_frq,
                    clk_div,
                )
                config_structs += '\n'

        elif IP_name == 'Xilinx':
            for i in range(0, len(data_dict['Name'])):
                name = str(data_dict['Name'][i])
                timer_no = str(data_dict['Timer NO.'][i])
                irq = str(data_dict['IRQ'][i])
                clk_frq = str(data_dict['Clock Frequency'][i])

                config_structs += TimerSrcFile.config_struct_template_xli % (
                    '%s_config' % name,
                    'DEVICE_BASE(%s)' % timer_no,
                    ('DEVICE_INTR(%s)' % timer_no) if irq == 'On' else '-1',
                    clk_frq,
                )
                config_structs += '\n'

        elif IP_name == 'BTM':
            for i in range(0, len(data_dict['Name'])):
                name = str(data_dict['Name'][i])
                timer_no = str(data_dict['Timer NO.'][i])
                irq = str(data_dict['IRQ'][i])

                config_structs += TimerSrcFile.config_struct_template_btm % (
                    '%s_config' % name,
                    'DEVICE_BASE(%s)' % timer_no,
                    ('DEVICE_INTR(%s_O_BTM)' % timer_no) if irq == 'On' else '-1',
                )
                config_structs += '\n'
        return config_structs

    def _timer_init_body_gen(self, data_dict, IP_name):
        timer_init_body = ''
        if IP_name == 'SemiDrive':
            for i in range(0, len(data_dict['Name'])):
                name = data_dict['Name'][i]
                dev_name = 'g_sdrv_%s' % name
                lowdev_name = 'g_%s' % name
                config_name = '%s_config' % name

                timer_init_body = '    sdrv_tmr_init(&%s, &%s);\n' % (
                    dev_name, config_name)
                timer_init_body += '    %s = sdrv_tmr_timer_probe(&%s, %s);\n' % (
                    lowdev_name, dev_name, data_dict['SemiDrive Timer ID.'][i])

        elif IP_name == 'Xilinx':
            for i in range(0, len(data_dict['Name'])):
                name = data_dict['Name'][i]
                dev_name = 'g_xli_%s' % name
                lowdev_name = 'g_%s' % name
                config_name = '%s_config' % name

                timer_init_body += '    %s = xli_ttc_probe(&%s, &%s);\n' % (
                    lowdev_name, dev_name, config_name)

        elif IP_name == 'BTM':
            for i in range(0, len(data_dict['Name'])):
                name = data_dict['Name'][i]
                dev_name = 'g_sdrv_btm_%s' % name
                lowdev_name = 'g_%s' % name
                config_name = '%s_config' % name

                timer_init_body += '    %s = sdrv_btm_probe(&%s, &%s);\n' % (
                    lowdev_name, dev_name, config_name)

        if data_dict['As Kernel Timer'][i] == 'Yes':
            timer_init_body += '    arch_timer_init(%s);\n' % lowdev_name
        timer_init_body += '\n'

        return timer_init_body

    def generate(self, excel):
        dictionary_list = []
        with PyxlExcelReader(excel) as excel_reader:
            IP_list = excel_reader.get_sheetnames()
            for IP in IP_list:
                dictionary_list.append(excel_reader.read_sheet_into_dict(IP))

        driver_headers = ''
        device_drivers = ''
        config_structs = ''
        timer_init_body = ''
        for idx, IP in enumerate(IP_list):
            data_dict = dictionary_list[idx]
            if len(data_dict['Name']) > 0:
                driver_headers += self._driver_headers_gen(IP)
                device_drivers += self._device_drivers_gen(data_dict, IP)
                config_structs += self._config_structs_gen(data_dict, IP)
                timer_init_body += self._timer_init_body_gen(data_dict, IP)

        timer_init_function = TimerSrcFile.timer_init_template % timer_init_body

        body = TimerSrcFile.body_template % (
            driver_headers,
            device_drivers,
            config_structs,
            timer_init_function
        )
        print(body)
        self.write(body)


# for timer_cfg.h file
class TimerIncFile(FileBase):
    prefix = FileBase.prefix + """
#ifndef BOARDS_TIMER_CFG_H
#define BOARDS_TIMER_CFG_H

#ifdef CONFIG_TIMER

#include <driver/timer/timer.h>
"""

    suffix = """
#endif /* BOARDS_TIMER_CFG_H */
"""

    body_template = """
/* exported timer devices */
%s
/* timer init functions */
extern void board_timer_init(void);

#endif
"""

    def _exported_declarations_gen(self, data_dict):
        exported_declarations = ''
        for i in range(0, len(data_dict['Name'])):
            name = data_dict['Name'][i]
            lowdev_name = 'g_%s' % name
            exported_declarations += 'extern timer_dev_t *%s;\n' % lowdev_name
        return exported_declarations

    def generate(self, excel):
        dictionary_list = []
        with PyxlExcelReader(excel) as excel_reader:
            IP_list = excel_reader.get_sheetnames()
            for IP in IP_list:
                dictionary_list.append(excel_reader.read_sheet_into_dict(IP))

        exported_declarations = ''
        for idx, IP in enumerate(IP_list):
            data_dict = dictionary_list[idx]
            if len(data_dict['Name']) > 0:
                exported_declarations += self._exported_declarations_gen(
                    data_dict)

        body = TimerIncFile.body_template % (
            exported_declarations
        )
        print(body)
        self.write(body)


def gen(project):
    with TimerSrcFile(project.get_generated_path('timer_cfg.c')) as src_file:
        excel_path = project.get_config_path('timer.dat')
        src_file.generate(excel_path)

    with TimerIncFile(project.get_generated_path('timer_cfg.h')) as inc_file:
        excel_path = project.get_config_path('timer.dat')
        inc_file.generate(excel_path)
