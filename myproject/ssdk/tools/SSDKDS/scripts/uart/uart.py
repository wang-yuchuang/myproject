from file_ops import FileBase
from excel_ops import PyxlExcelReader
import project

# for uart_cfg.c file
class UartSrcFile(FileBase):
    body_template = \
"""
#ifdef CONFIG_UART

#include "uart_cfg.h"

#include <types.h>
#include <board.h>

/* uart device driver headers */
%s
/* uart device drivers */
%s
/* uart config structures */
%s
/* uart early initialization */
%s
/* uart initialization */
%s

#endif
"""

    config_struct_template = \
"""static const uart_config_t %s = {
    .base = %s,
    .irq = %s,
    .baud = %s,
    .clk_freq = %s,
    .data_bits = %s,
    .stop_bits = %s,
    .parity = %s,
};
"""

    early_init_template = \
"""void board_uart_early_init(void)
{
%s}
"""

    uart_init_template = \
"""void board_uart_init(void)
{
%s}
"""

    def _driver_headers_gen(self, IP_name):
        driver_headers = ''
        if IP_name == 'SemiDrive':
            driver_headers += '#include <driver/uart/sdrv_uart_drv.h>\n'
        elif IP_name == 'DesignWare':
            driver_headers += '#include <driver/uart/dw_uart_drv.h>\n'
        elif IP_name == 'Xilinx':
            driver_headers += '#include <driver/uart/xli_uart_drv.h>\n'
        return driver_headers

    def _device_drivers_gen(self, data_dict, IP_name):
        device_drivers = ''
        if IP_name == 'SemiDrive':
            for i in range(0, len(data_dict['Name'])):
                name = data_dict['Name'][i]
                dev_name = 'g_sdrv_%s' % name
                lowdev_name = 'g_%s' % name

                device_drivers += \
                    'sdrv_uart_dev_t %s;\n' % dev_name
                device_drivers += \
                    'uart_dev_t *%s;\n\n' % lowdev_name

        elif IP_name == 'DesignWare':
            for i in range(0, len(data_dict['Name'])):
                name = data_dict['Name'][i]
                dev_name = 'g_dw_%s' % name
                lowdev_name = 'g_%s' % name

                device_drivers += \
                    'dw_uart_dev_t %s;\n' % dev_name
                device_drivers += \
                    'uart_dev_t *%s;\n\n' % lowdev_name

        elif IP_name == 'Xilinx':
            for i in range(0, len(data_dict['Name'])):
                name = data_dict['Name'][i]
                dev_name = 'g_xli_%s' % name
                lowdev_name = 'g_%s' % name

                device_drivers += \
                    'xli_uart_dev_t %s;\n' % dev_name
                device_drivers += \
                    'uart_dev_t *%s;\n\n' % lowdev_name
        return device_drivers

    def _config_structs_gen(self, data_dict):
        config_structs = ''
        for i in range(0, len(data_dict['Name'])):
            name = str(data_dict['Name'][i])
            uart_no = str(data_dict['UART NO.'][i])
            irq = str(data_dict['IRQ'][i])
            baud = str(data_dict['Baudrate'][i])
            clk_freq = str(data_dict['Clock Frequency'][i])
            data_bits = str(data_dict['Data Bits'][i])
            stop_bits = str(data_dict['Stop Bits'][i])
            parity = str(data_dict['Parity'][i])

            config_structs += UartSrcFile.config_struct_template % (
                '%s_config' % name,
                'DEVICE_BASE(%s)' % uart_no,
                ('DEVICE_INTR(%s)' % uart_no) if irq == 'On' else '-1',
                baud,
                clk_freq,
                'UART_CHAR_%s_BITS' % data_bits,
                'UART_STOP_%s_BIT' % stop_bits.replace('.', '_'),
                'UART_%s_PARITY' % parity.upper(),
            )
            config_structs += '\n'
        return config_structs

    def _early_init_body_gen(self, data_dict, IP_name):
        early_init_body = ''
        if IP_name == 'SemiDrive':
            for i in range(0, len(data_dict['Name'])):
                name = data_dict['Name'][i]
                dev_name = 'g_sdrv_%s' % name
                lowdev_name = 'g_%s' % name
                config_name = '%s_config' % name

                early_init_body += '    %s = sdrv_uart_probe(&%s, &%s);\n' % (
                    lowdev_name, dev_name, config_name)
                if data_dict['As Console UART'][i] == 'Yes':
                    early_init_body += '    board_set_console(%s);\n' % lowdev_name
                early_init_body += '\n'

        elif IP_name == 'DesignWare':
            for i in range(0, len(data_dict['Name'])):
                name = data_dict['Name'][i]
                dev_name = 'g_dw_%s' % name
                lowdev_name = 'g_%s' % name
                config_name = '%s_config' % name

                early_init_body += '    %s = dw_uart_probe(&%s, &%s);\n' % (
                    lowdev_name, dev_name, config_name)
                if data_dict['As Console UART'][i] == 'Yes':
                    early_init_body += '    board_set_console(%s);\n' % lowdev_name
                early_init_body += '\n'

        elif IP_name == 'Xilinx':
            for i in range(0, len(data_dict['Name'])):
                name = data_dict['Name'][i]
                dev_name = 'g_xli_%s' % name
                lowdev_name = 'g_%s' % name
                config_name = '%s_config' % name

                early_init_body += '    %s = xli_uart_probe(&%s, &%s);\n' % (
                    lowdev_name, dev_name, config_name)
                if data_dict['As Console UART'][i] == 'Yes':
                    early_init_body += '    board_set_console(%s);\n' % lowdev_name
                early_init_body += '\n'
        return early_init_body

    def _uart_init_body_gen(self, data_dict):
        uart_init_body = ''
        for i in range(0, len(data_dict['Name'])):
            name = data_dict['Name'][i]
            lowdev_name = 'g_%s' % name
            uart_init_body = '    uart_init(%s);\n' % lowdev_name

        return uart_init_body

    def generate(self, excel):
        dictionary_list = []
        with PyxlExcelReader(excel) as excel_reader:
            IP_list = excel_reader.get_sheetnames()
            for IP in IP_list:
                dictionary_list.append(excel_reader.read_sheet_into_dict(IP))

        driver_headers = ''
        device_drivers = ''
        config_structs = ''
        early_init_body = ''
        uart_init_body = ''
        for idx, IP in enumerate(IP_list):
            data_dict = dictionary_list[idx]
            if len(data_dict['Name']) > 0:
                driver_headers += self._driver_headers_gen(IP)
                device_drivers += self._device_drivers_gen(data_dict, IP)
                config_structs += self._config_structs_gen(data_dict)
                early_init_body += self._early_init_body_gen(data_dict, IP)
                uart_init_body += self._uart_init_body_gen(data_dict)

        early_init_function = UartSrcFile.early_init_template % early_init_body
        uart_init_function = UartSrcFile.uart_init_template % uart_init_body

        body = UartSrcFile.body_template % (
            driver_headers,
            device_drivers,
            config_structs,
            early_init_function,
            uart_init_function
        )
        print(body)
        self.write(body)


# for uart_cfg.h file
class UartIncFile(FileBase):
    prefix = FileBase.prefix + """
#ifndef BOARDS_UART_CFG_H
#define BOARDS_UART_CFG_H

#ifdef CONFIG_UART

#include <driver/uart/uart.h>
"""

    suffix = """
#endif /* BOARDS_UART_CFG_H */
"""

    body_template = """
/* exported uart devices */
%s
/* uart init functions */
extern void board_uart_early_init(void);
extern void board_uart_init(void);

#endif
"""

    def _exported_declarations_gen(self, data_dict):
        exported_declarations = ''
        for i in range(0, len(data_dict['Name'])):
            name = data_dict['Name'][i]
            lowdev_name = 'g_%s' % name
            exported_declarations += 'extern uart_dev_t *%s;\n' % lowdev_name
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
                exported_declarations += self._exported_declarations_gen(data_dict)

        body = UartIncFile.body_template % (
            exported_declarations
        )
        print(body)
        self.write(body)


def gen(project):
    with UartSrcFile(project.get_generated_path('uart_cfg.c')) as src_file:
        excel_path = project.get_config_path('uart.dat')
        src_file.generate(excel_path)

    with UartIncFile(project.get_generated_path('uart_cfg.h')) as inc_file:
        excel_path = project.get_config_path('uart.dat')
        inc_file.generate(excel_path)
