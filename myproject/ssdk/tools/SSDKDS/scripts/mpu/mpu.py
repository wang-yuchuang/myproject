from file_ops import FileBase
from excel_ops import PyxlExcelReader
import project

# for mpu_cfg.h file
class MpuIncFile(FileBase):
    prefix = FileBase.prefix + """
#ifndef BOARDS_MPU_CFG_H
#define BOARDS_MPU_CFG_H

#ifdef CONFIG_ARCH_USE_MPU

extern void board_mpu_init(void);

"""

    suffix = """
#endif

#endif /* BOARDS_MPU_CFG_H */
"""


# for mpu_cfg.c file
class MpuSrcFile(FileBase):
    prefix = FileBase.prefix + """
#ifdef CONFIG_ARCH_USE_MPU

#include <types.h>
#include <param.h>
#include <arch/armv7-r/mpu.h>

#include "mpu_cfg.h"

"""

    config_struct_template = """
const mpu_config_t board_mpu_config[] = {%s
};
"""

    struct_item_template = """
    {%s, %s, %s},"""

    suffix = """
void board_mpu_init(void)
{
    uint32_t index = 0;

    mpu_enable(false);

    for (uint32_t i = 0; i < ARRAY_SIZE(board_mpu_config); i++) {
        const mpu_config_t *config = &board_mpu_config[i];
        if (config->size > 0) {
            mpu_add_region(index++, config->addr, config->size, config->type);
        }
    }

    if (index > 0) {
        mpu_enable(true);
    }
}

#endif
"""

def struct_gen(config_struct_template, struct_item_template, data_dict):
    config_items = ''
    if len(data_dict['Address']) == 0:
        config_items = """
    {0},"""
    else:
        for i in range(0, len(data_dict['Address'])):
            address = str(data_dict['Address'][i])
            size = str(data_dict['Size'][i])
            type = str(data_dict['Type'][i])
            comment = data_dict['Comment'][i]  # in case None is casted to 'None'

            if comment is not None:
                config_items += """
    /* %s */"""%comment

            config_items += struct_item_template % (
                address,
                size,
                'MPU_REGION_%s' % type.replace(' ', '_').upper(),
            )

    config_struct = config_struct_template % config_items
    return config_struct


def gen(project):
    excel_reader = PyxlExcelReader(project.get_config_path('mpu.dat'))

    data_dict = excel_reader.read_sheet_into_dict('MPU')
    config_struct = struct_gen(MpuSrcFile.config_struct_template,
                               MpuSrcFile.struct_item_template, data_dict)

    with MpuIncFile(project.get_generated_path('mpu_cfg.h')) as out_file:
        out_file.write('')

    with MpuSrcFile(project.get_generated_path('mpu_cfg.c')) as out_file:
        out_file.write(config_struct)

