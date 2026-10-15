OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_start)
SECTIONS
{
    . = MEMBASE;

    _start = .;

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

    /* List sections that have differet load address and link address,
     * requiring early section copy on start up.
     */
    .earlycopy : ALIGN(4) {
        __earlycopy_start = .;

        /* TCM code and data section. */
        LONG(ADDR(.tcm))
        LONG(SIZEOF(.tcm))
        LONG(LOADADDR(.tcm))
		
        /* Add other sections here. */
		
		/* IRAM code and data section. */
		LONG(ADDR(.iram))
        LONG(SIZEOF(.iram))
        LONG(LOADADDR(.iram))
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

    .nocache : ALIGN(1024) {
        _nocacheable_start = .;
        *(.nocache)
        . = ALIGN(1024);
        _nocacheable_end = .;
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
    .except.stack : ALIGN(4) {
        __except_stack_start = .;
        *(.except.stack)
        __except_stack_end = .;
    }

    _end = .;
	
	
	/* TCMA=0x4C0000 TCMB=0x4B0000 0x3722b000*/
    /* TCM code and data sections.__data_end 0x0000000037237864*/
    .tcm 0x4B0000 : AT (_end) {
        __tcm_start = .;
        *(.tcm_func*)
		. = ALIGN(4);
		*(.tcm_data*)
        __tcm_end = ALIGN(4);
		/*ASSERT(__tcm_end - __tcm_start <= 0x10000, "TCM overflow!");*/
    }

	_iram_ddr_start = _end + __tcm_end - __tcm_start;
	  .iram 0x100000  : AT (_iram_ddr_start) {
        __iram_start = .;
        *(.iram_func*)
		. = ALIGN(4);
		*(.iram_data*)
        __iram_end = ALIGN(4);
    }

    . = _end + __tcm_end - __tcm_start + __iram_end - __iram_start;
    .heap (NOLOAD) : ALIGN(4) {
        _heap_start = .;
        *(.heap)
    }

    . = MEMBASE + MEMSIZE;

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

    /* Strip unnecessary stuff */
    /DISCARD/ : { *(.comment .note .eh_frame) }
}

