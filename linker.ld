ENTRY (_loader)

SECTIONS{
    . = 0x00100000;

	_start = .;
    .text :{
        *(.text)
        *(.rodata)
    }

    .data ALIGN (0x1000) : {
        *(.data)
    }

    .bss : {
        _sbss = .;
        *(COMMON)
        *(.bss)
        _ebss = .;
    }
	_end = .;
}
