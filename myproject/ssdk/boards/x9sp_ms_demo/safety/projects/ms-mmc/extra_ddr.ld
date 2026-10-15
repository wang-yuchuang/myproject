#include "image_cfg.h"

SECTIONS {
    . = SAF_MEMBASE;

    LMA_EXTRA_DDR_DATA = LOADADDR(.tcmdata) + SIZEOF(.tcmdata);
    .extra_ddr_data : AT(LMA_EXTRA_DDR_DATA) {
        __extra_ddr_data_start = .;
        *(.ddr_data)
        __extra_ddr_data_end = .;
    }
    __extra_ddr_data_lma = LOADADDR(.extra_ddr_data);
}
INSERT AFTER .tcm.excpet.stack;
