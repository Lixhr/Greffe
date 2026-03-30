MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 248K
    PATCH (rx)  : ORIGIN = 0x00010000, LENGTH = 8K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

_stack_top = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    .text :
    {
        KEEP(*(.vectors))
        *(.text*)
        *(.rodata*)
    } > FLASH

    .bss (NOLOAD) :
    {
        *(.bss*)
        *(COMMON)
    } > RAM

    .data :
    {
        *(.data*)
    } > RAM AT > FLASH


    .patch :
    {
        patch_base = .;
        FILL(0xFF);
        . += LENGTH(PATCH);
        patch_end = .;
    } > PATCH
}
