ENTRY (_loader)

SECTIONS{
    . = 0x00100000;
	_phys_start = .;
	.setup :
	{
		*(.setup)
	}

	. += 0xC0000000;
	_start = .;
    .text : AT(ADDR(.text) - 0xC0000000)
	{
        *(.text)
        *(.rodata*)
    }

    .data ALIGN(0x1000) : AT(ADDR(.data) - 0xC0000000)
	{
        *(.data)
    }

    .bss ALIGN(0x1000) : AT(ADDR(.bss) - 0xC0000000)
	{
        _sbss = .;
        *(COMMON)
        *(.bss)
        _ebss = .;
    }
	_end = .;
	/* We really don't care which tools were used to make the binary :/ */
	/DISCARD/ : { *(.comment) }
}
