/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 sulfurLabs
 *
 * PROJECT: s4
 * FILE: user.ld
 *
 */

OUTPUT_FORMAT( elf64-x86-64 )
OUTPUT_ARCH( i386:x86-64 )

ENTRY(
	_start
)
PHDRS {
	load PT_LOAD FLAGS(7);
}
SECTIONS
{
    . = 0x40000000;

    .text :
    {
    	*(.text._start)
     	*(.text*)
    } :load

    .rodata :
    {
    	*(.rodata*)
    } :load

    .data :
    {
    	*(.data*)
    } :load

    .bss :
    {
    	*(.bss*)
     	*(COMMON)
    } :load

    /DISCARD/ :
    {
    	*(.eh_frame)
     	*(.note*)
      	*(.comment)
    }
}