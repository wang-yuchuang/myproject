OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_start)
SECTIONS
{
    _start = MEMBASE;

    . = MEMBASE;

    /* exception vector table in tcm, if tcma base is set to non-zero,
     * this output section will be empty.
     */
    .text.vectab : {
        KEEP(*(.text.vectab))
    }

    /* text/read-only data */
    .text : {
        KEEP(*(.text.boot.vectab1))
        KEEP(*(.text.boot.vectab2))
        KEEP(*(.text.boot))
        *(.text* .sram.text.glue_7* .gnu.linkonce.t.*)
    }

    .interp : { *(.interp) }
    .hash : { *(.hash) }
    .dynsym : { *(.dynsym) }
    .dynstr : { *(.dynstr) }
    .rel.text : { *(.rel.text) *(.rel.gnu.linkonce.t*) }
    .rela.text : { *(.rela.text) *(.rela.gnu.linkonce.t*) }
    .rel.data : { *(.rel.data) *(.rel.gnu.linkonce.d*) }
    .rela.data : { *(.rela.data) *(.rela.gnu.linkonce.d*) }
    .rel.rodata : { *(.rel.rodata) *(.rel.gnu.linkonce.r*) }
    .rela.rodata : { *(.rela.rodata) *(.rela.gnu.linkonce.r*) }
    .rel.got : { *(.rel.got) }
    .rela.got : { *(.rela.got) }
    .rel.ctors : { *(.rel.ctors) }
    .rela.ctors : { *(.rela.ctors) }
    .rel.dtors : { *(.rel.dtors) }
    .rela.dtors : { *(.rela.dtors) }
    .rel.init : { *(.rel.init) }
    .rela.init : { *(.rela.init) }
    .rel.fini : { *(.rel.fini) }
    .rela.fini : { *(.rela.fini) }
    .rel.bss : { *(.rel.bss) }
    .rela.bss : { *(.rela.bss) }
    .rel.plt : { *(.rel.plt) }
    .rela.plt : { *(.rela.plt) }
    .init : { *(.init) } =0x9090
    .plt : { *(.plt) }

    /* .ARM.exidx is sorted, so has to go in its own output section.  */
    __exidx_start = .;
    .ARM.exidx : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
    __exidx_end = .;

    .dummy_post_text : {
        __code_end = .;
    }

    .rodata : ALIGN(4) {
        __rodata_start = .;
        __fault_handler_table_start = .;
        KEEP(*(.rodata.fault_handler_table))
        __fault_handler_table_end = .;
        *(.rodata .rodata.* .gnu.linkonce.r.*)
    }

    /*
     * extra linker scripts tend to insert sections just after .rodata,
     * so we want to make sure this symbol comes after anything inserted above,
     * but not aligned to the next section necessarily.
     */
    .dummy_post_rodata : {
        __rodata_end = .;
    }

    .data : ALIGN(4) {
        /* writable data  */
        __data_start_rom = .;
        /* in one segment binaries, the rom data address is on top of the ram data address */
        __data_start = .;
        *(.data .data.* .gnu.linkonce.d.*)
    }

    .ctors : ALIGN(4) {
        __ctor_list = .;
        KEEP(*(.ctors .init_array))
        __ctor_end = .;
    }
    .dtors : ALIGN(4) {
        __dtor_list = .;
        KEEP(*(.dtors .fini_array))
        __dtor_end = .;
    }
    .got : { *(.got.plt) *(.got) }
    .dynamic : { *(.dynamic) }

    .nocache : ALIGN(1024) {
        _nocacheable_start = .;
        *(.nocache)
        . = ALIGN(1024);
        _nocacheable_end = .;
    }

    .earlycopy : ALIGN(4) {
        __earlycopy_start = .;

        LONG(TCMA_BASE)
        LONG(SIZEOF(.text.vectab))
        LONG(LOADADDR(.text.vectab))

        LONG(ADDR(.tcmcode))
        LONG(SIZEOF(.tcmcode))
        LONG(LOADADDR(.tcmcode))

        LONG(ADDR(.tcmdata))
        LONG(SIZEOF(.tcmdata))
        LONG(LOADADDR(.tcmdata))

        __earlycopy_end = .;
    }

    /*
     * extra linker scripts tend to insert sections just after .data,
     * so we want to make sure this symbol comes after anything inserted above,
     * but not aligned to the next section necessarily.
     */
    .dummy_post_data : {
        __data_end = .;
    }

    /* uninitialized data (in same segment as writable data) */
    .bss : ALIGN(4) {
        KEEP(*(.bss.prebss.*))
        . = ALIGN(4);
        __bss_start = .;
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    }

    /* exception stack */
    .except.stack (NOLOAD) : ALIGN(4) {
        __except_stack_start = .;
        *(.except.stack)
        __except_stack_end = .;
    }

    _end = .;

    .heap (NOLOAD) : ALIGN(4) {
        _heap_start = .;
        *(.heap)
    }

    . = MEMBASE + MEMSIZE - RESSIZE;

    _heap_end = .;

    _end_of_ram = .;

    /* Add a labelize section of ext memory start addr, which could
     * be locate to extra linker script */
    .ext.mem (NOLOAD) : {
        . = ALIGN(4);
        __ext_mem_start = .;
        *(.ext.mem .ext.mem.*)
        . = ALIGN(4);
        __ext_mem_end = .;

    }

    . = ALIGN(TCMA_BASE + SIZEOF(.text.vectab), 4);
    .tcmcode : AT(LOADADDR(.bss)) {
        *(.tcmcode)
        ASSERT(. < TCMA_BASE + 0x10000, "TCMA image too large!");
    }

    . = TCMB_BASE;
    LMA_TCMDATA = LOADADDR(.tcmcode) + SIZEOF(.tcmcode);
    .tcmdata : AT(LMA_TCMDATA) {
        *(.tcmdata)
    }

    .tcm.excpet.stack (NOLOAD) : ALIGN(8) {
        __tcm_except_stack_start = .;
        *(.tcm.except.stack)
        __tcm_except_stack_end = .;
        ASSERT(. < TCMB_BASE + 0x10000, "TCMB image too large!");
    }

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note .eh_frame) }
}

