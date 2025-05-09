/*
 * SPDX-FileCopyrightText: 2023 Ledger SAS
 * SPDX-License-Identifier: Apache-2.0
 */

ENTRY(_start)

MEMORY
{
    APP_CODE (rx): ORIGIN = 0x08000000, LENGTH = 0x00040000
    APP_RAM (rw): ORIGIN = 0x20000000, LENGTH = 0x00002000
}

/* Define output sections */
SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        _stext = .;	            /* create a global symbol at data start */
        *(.text._start*)
        *(.text._exit*)
        *(.text)
        *(.text*)
        *(.rodata)         	/* .rodata sections (constants, strings, etc.) */
        *(.rodata*)         	/* .rodata sections (constants, strings, etc.) */
        *(.glue_7)         	/* glue arm to thumb code */
        *(.glue_7t)        	/* glue thumb to arm code */
        *(.eh_frame)
        KEEP (*(.init))
        KEEP (*(.fini))
        . = ALIGN(4);

        _exit = .;
    } > APP_CODE

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > APP_CODE

    .ARM :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > APP_CODE

    .ctors :
    {
        __CTOR_LIST__ = .;
        /* gcc uses crtbegin.o to find the start of
        the constructors, so we make sure it is
        first.  Because this is a wildcard, it
        doesn't matter if the user does not
        actually link against crtbegin.o; the
        linker won't look for a file to match a
        wildcard.  The wildcard also means that it
        doesn't matter which directory crtbegin.o
        is in.  */
        KEEP (*crtbegin.o(.ctors))
        KEEP (*crtbegin?.o(.ctors))
        /* We don't want to include the .ctor section from
        from the crtend.o file until after the sorted ctors.
        The .ctor section from the crtend file contains the
        end of ctors marker and it must be last */
        KEEP (*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*(.ctors))
        __CTOR_END__ = .;
    } > APP_CODE

    _etext = .;        	/* define a global symbols at end of code */
    _sigot = ALIGN(4);
    . = . + SIZEOF(.got);
    _sidata = ALIGN(4);
    . = . + SIZEOF(.data);
    _erom = .;

    .svcexchange :
    {
        . = ALIGN(4);
        _s_svcexchange = .;
        KEEP(*(.svcexchange*))
        _e_svcexchange = .;
    } > APP_RAM

    /* used by the startup to initialize got */
    .got : AT ( _sigot )
    {
        . = ALIGN(4);
            _sgot = .;
            /*  *(.got.plt)
            *    We don't need plt segment
            *    since we do not need dynamic library relocation
            */
            *(.got)
            *(.got*)
        . = ALIGN(4);
        _egot = .;
    } > APP_RAM

    /* used by the startup to initialize data */

    /* used by the startup to initialize data */
    /* Initialized data sections goes into RAM, load LMA copy after code */
    .data : AT ( _sidata )
    {
        . = ALIGN(4);
        _ram_start = .;
        _sdata = .;        /* create a global symbol at data start */
        *(.data)           /* .data sections */
        *(.data*)          /* .data* sections */
        _edata = .;        /* define a global symbol at data end */
    } > APP_RAM

    /* Uninitialized data section */
    . = ALIGN(4);
    .bss :
    {
        /* This is used by the startup in order to initialize the .bss section */
        _sbss = .;         /* define a global symbol at bss start */
        __bss_start__ = _sbss;
        *debug.o(.bss)
        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;         /* define a global symbol at bss end */
        __bss_end__ = _ebss;
    } > APP_RAM

    /*
     * Those symbols define heap start and end addresses, no heap by default
     * if user add heap in it's task config, _eheap sym is patched at relocation
     * time.
     */
    _sheap = .;
    _eheap = .;
}
