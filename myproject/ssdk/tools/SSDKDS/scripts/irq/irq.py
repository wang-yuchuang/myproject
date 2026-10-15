#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import sys
import re
sys.path.append(os.path.dirname(__file__))
from file_ops import FileBase
import project

# for irq_num.h file
class IrqIncFile(FileBase):
    prefix = FileBase.STATEMENT + """
#ifndef IRQ_NUM_H
#define IRQ_NUM_H

"""

    suffix = """

#endif /* IRQ_NUM_H */
"""

def gen(project):
    gic_id = project['gic']
    config_file = open(project.get_config_path('irq.dat'))
    with IrqIncFile(project.get_generated_path('irq_num.h')) as irq_file:
        irq_file.write("#define {: <60} {}\n".format('INT_BASE', '%s_BASE' % (gic_id)))
        for line in config_file.readlines():
            line = line.strip('\n')
            search_obj = re.search(r'\s*#define \s*(.*?)_(.IC[1-9].*?)_(.*) \s*(.*)', line, re.I)
            if search_obj:
                '''
                search_obj.group(1) : IRQ
                search_obj.group(2) : GIC1
                search_obj.group(3) : DMA1_INTR_NUM
                '''
                if search_obj.group(2) == gic_id:
                    code = "#define {: <60} {}\n".format(search_obj.group(3), search_obj.group(4))
                    irq_file.write(code)

