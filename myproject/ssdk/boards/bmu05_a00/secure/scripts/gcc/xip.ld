OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_start)
SECTIONS
{
    . = MEMBASE;

    _start = .;

    /* text/read-only data — XIP: 在 OSPI2 NOR Flash 原地执行 (design.md D1, D2)
     * MEMBASE = 0x08000000 (OSPI2_MEM 基地址)
     * A/B 两槽通过 SCU OSPI2 remap 统一映射到此地址 (design.md D2) */
    .text : {
        KEEP(*(.text.boot.vectab1))
        KEEP(*(.text.boot.vectab2))
        KEEP(*(.text.boot))
        *(.text* .sram.text.glue_7* .gnu.linkonce.t.*)
    }

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

    /* List sections that have different load address and link address,
     * requiring early section copy on start up.
     *
     * NOTE (design.md D9): earlycopy 表描述了哪些段需要从 NOR (LMA) 拷贝到
     * TCM/SRAM (VMA)。startup 代码本身通过 __tcmcode 属性放在 .tcm 段（由
     * reset vector 第一阶段拷贝到 TCM 执行），避免 earlycopy 期间 OSPI 总线
     * 竞争（取指与数据读同时走 AHB）。
     */
    .earlycopy : ALIGN(4) {
        __earlycopy_start = .;

        /* TCM code and data section. */
        LONG(ADDR(.tcm))
        LONG(SIZEOF(.tcm))
        LONG(LOADADDR(.tcm))

        /* RAM code section. */
        LONG(ADDR(.ram.text))
        LONG(SIZEOF(.ram.text))
        LONG(LOADADDR(.ram.text))

        /* RAM data section. */
        LONG(ADDR(.data))
        LONG(SIZEOF(.data))
        LONG(LOADADDR(.data))

        __earlycopy_end = .;
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

    __rom_end = .;

    /* TCM code and data sections.
     * VMA 在 TCM (TCMBASE)，LMA 在 NOR Flash (__rom_end 之后)。
     * 由 earlycopy 机制在启动时从 NOR 拷贝到 TCM。
     * 中断向量表、中断处理函数、earlycopy 代码本身放这里 (design.md D9)。
     */
    .tcm TCMBASE : AT ( __rom_end ) ALIGN(4) {
        __tcm_start = .;
        *(.tcmcode)

        . = ALIGN(4);
        *(.tcmdata)

        __tcm_end = ALIGN(4);
        ASSERT(__tcm_end - __tcm_start <= TCMSIZE, "TCM overflow!");
    }

    /* place R/W data and some code which can't xip in the RAM */
    . = RAM_BASE;

    .data : AT (LOADADDR(.tcm) + SIZEOF (.tcm)) ALIGN(4) {
        /* in one segment binaries, the rom data address is on top of the ram data address */
        __data_start = .;
        *(.data .data.* .gnu.linkonce.d.*)
    }

    /* code that is located in ram — NOR Flash 擦写函数等必须放 RAM 运行的代码 */
    .ram.text : ALIGN(4) {

        KEEP (*(.ram.text*))
    }

    /*
     * extra linker scripts tend to insert sections just after .data,
     * so we want to make sure this symbol comes after anything inserted above,
     * but not aligned to the next section necessarily.
     */
    .dummy_post_data : {
        /* End of initialized sections. */
        __data_end = .;
    }

    .nocache : ALIGN(1024) {
        _nocacheable_start = .;
        KEEP(*(.nocache))
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

    .heap : ALIGN(4) {
        _heap_start = .;
        *(.heap)
    }

    . = RAM_BASE + RAM_SIZE;

    _heap_end = .;

    _end_of_ram = .;

    /* IRAM none-ecc data sections. */
    .noneecc NONEECCBASE (NOLOAD) : ALIGN(4) {
        _noneecc_start = .;
        *(.noneeccdata)
    }

    . = NONEECCBASE + NONEECCSIZE;

    _noneecc_end = .;

    /* Add a labelized section of ext memory start addr, which could
     * be located to extra linker script */
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
