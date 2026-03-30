.syntax unified
.thumb

.section .text.Reset_Handler
.thumb_func
.global Reset_Handler
Reset_Handler:
    bl   main
    b    .
