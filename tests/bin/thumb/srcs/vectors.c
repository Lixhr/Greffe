extern unsigned int _stack_top;

void Reset_Handler(void);
void Default_Handler(void)  { while (1); }
void HardFault_Handler(void);   /* fault.c */
void MemManage_Handler(void);   /* fault.c */

__attribute__((section(".vectors"), used))
void (* const vector_table[])(void) = {
    (void (*)(void))&_stack_top,  /* Initial stack pointer */
    Reset_Handler,                /* Reset                 */
    Default_Handler,              /* NMI                   */
    HardFault_Handler,            /* Hard fault            */
    MemManage_Handler,            /* Memory manage (MPU)   */
    Default_Handler,              /* Bus fault             */
    Default_Handler,              /* Usage fault           */
    0, 0, 0, 0,                   /* Reserved              */
    Default_Handler,              /* SVCall                */
    0, 0,                         /* Reserved              */
    Default_Handler,              /* PendSV                */
    Default_Handler,              /* SysTick               */
};
