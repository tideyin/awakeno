/*
Startup file for GD32E10x
*/

#include <stdint.h>

extern uint32_t _sdata, _edata, _rdata, _sbss, _ebss, _estack;

void main(void);

void Reset_Handler(void)
{
	uint32_t *src, *dst;
	src=&_rdata;
	dst=&_sdata;
	while(dst<&_edata) *dst++=*src++;
	dst=&_sbss;
	while(dst<&_ebss) *dst++=0;
	main();
	while(1);
}

void Null_Handler(void) {return;}
void Loop_Handler(void) {while(1);}

//Dummy interrupt handlers
void __attribute__((weak, alias ("Null_Handler"))) NMI_Handler                       (void); //NMI Handler
void __attribute__((weak, alias ("Loop_Handler"))) HardFault_Handler                 (void); //Hard Fault Handler
void __attribute__((weak, alias ("Loop_Handler"))) MemManage_Handler                 (void); //MPU Fault Handler
void __attribute__((weak, alias ("Loop_Handler"))) BusFault_Handler                  (void); //Bus Fault Handler
void __attribute__((weak, alias ("Loop_Handler"))) UsageFault_Handler                (void); //Usage Fault Handler
void __attribute__((weak, alias ("Null_Handler"))) SVC_Handler                       (void); //SVCall Handler
void __attribute__((weak, alias ("Null_Handler"))) DebugMon_Handler                  (void); //Debug Monitor Handler
void __attribute__((weak, alias ("Null_Handler"))) PendSV_Handler                    (void); //PendSV Handler
void __attribute__((weak, alias ("Null_Handler"))) SysTick_Handler                   (void); //SysTick Handler

//              /* external interrupts handler */
void __attribute__((weak, alias ("Null_Handler"))) WWDGT_IRQHandler                  (void); //16:Window Watchdog Timer
void __attribute__((weak, alias ("Null_Handler"))) LVD_IRQHandler                    (void); //17:LVD through EXTI Line detect
void __attribute__((weak, alias ("Null_Handler"))) TAMPER_IRQHandler                 (void); //18:Tamper through EXTI Line detect
void __attribute__((weak, alias ("Null_Handler"))) RTC_IRQHandler                    (void); //19:RTC through EXTI Line
void __attribute__((weak, alias ("Null_Handler"))) FMC_IRQHandler                    (void); //20:FMC
void __attribute__((weak, alias ("Null_Handler"))) RCU_CTC_IRQHandler                (void); //21:RCU and CTC
void __attribute__((weak, alias ("Null_Handler"))) EXTI0_IRQHandler                  (void); //22:EXTI Line 0
void __attribute__((weak, alias ("Null_Handler"))) EXTI1_IRQHandler                  (void); //23:EXTI Line 1
void __attribute__((weak, alias ("Null_Handler"))) EXTI2_IRQHandler                  (void); //24:EXTI Line 2
void __attribute__((weak, alias ("Null_Handler"))) EXTI3_IRQHandler                  (void); //25:EXTI Line 3
void __attribute__((weak, alias ("Null_Handler"))) EXTI4_IRQHandler                  (void); //26:EXTI Line 4
void __attribute__((weak, alias ("Null_Handler"))) DMA0_Channel0_IRQHandler          (void); //27:DMA0 Channel0
void __attribute__((weak, alias ("Null_Handler"))) DMA0_Channel1_IRQHandler          (void); //28:DMA0 Channel1
void __attribute__((weak, alias ("Null_Handler"))) DMA0_Channel2_IRQHandler          (void); //29:DMA0 Channel2
void __attribute__((weak, alias ("Null_Handler"))) DMA0_Channel3_IRQHandler          (void); //30:DMA0 Channel3
void __attribute__((weak, alias ("Null_Handler"))) DMA0_Channel4_IRQHandler          (void); //31:DMA0 Channel4
void __attribute__((weak, alias ("Null_Handler"))) DMA0_Channel5_IRQHandler          (void); //32:DMA0 Channel5
void __attribute__((weak, alias ("Null_Handler"))) DMA0_Channel6_IRQHandler          (void); //33:DMA0 Channel6
void __attribute__((weak, alias ("Null_Handler"))) ADC0_1_IRQHandler                 (void); //34:ADC0 and ADC1
void __attribute__((weak, alias ("Null_Handler"))) EXTI5_9_IRQHandler                (void); //39:EXTI5 to EXTI9
void __attribute__((weak, alias ("Null_Handler"))) TIMER0_BRK_TIMER8_IRQHandler      (void); //40:TIMER0 Break and TIMER8
void __attribute__((weak, alias ("Null_Handler"))) TIMER0_UP_TIMER9_IRQHandler       (void); //41:TIMER0 Update and TIMER9
void __attribute__((weak, alias ("Null_Handler"))) TIMER0_TRG_CMT_TIMER10_IRQHandler (void); //42:TIMER0 Trigger and Commutation and TIMER10
void __attribute__((weak, alias ("Null_Handler"))) TIMER0_Channel_IRQHandler         (void); //43:TIMER0 Channel Capture Compare
void __attribute__((weak, alias ("Null_Handler"))) TIMER1_IRQHandler                 (void); //44:TIMER1
void __attribute__((weak, alias ("Null_Handler"))) TIMER2_IRQHandler                 (void); //45:TIMER2
void __attribute__((weak, alias ("Null_Handler"))) TIMER3_IRQHandler                 (void); //46:TIMER3
void __attribute__((weak, alias ("Null_Handler"))) I2C0_EV_IRQHandler                (void); //47:I2C0 Event
void __attribute__((weak, alias ("Null_Handler"))) I2C0_ER_IRQHandler                (void); //48:I2C0 Error
void __attribute__((weak, alias ("Null_Handler"))) I2C1_EV_IRQHandler                (void); //49:I2C1 Event
void __attribute__((weak, alias ("Null_Handler"))) I2C1_ER_IRQHandler                (void); //50:I2C1 Error
void __attribute__((weak, alias ("Null_Handler"))) SPI0_IRQHandler                   (void); //51:SPI0
void __attribute__((weak, alias ("Null_Handler"))) SPI1_IRQHandler                   (void); //52:SPI1
void __attribute__((weak, alias ("Null_Handler"))) USART0_IRQHandler                 (void); //53:USART0
void __attribute__((weak, alias ("Null_Handler"))) USART1_IRQHandler                 (void); //54:USART1
void __attribute__((weak, alias ("Null_Handler"))) USART2_IRQHandler                 (void); //55:USART2
void __attribute__((weak, alias ("Null_Handler"))) EXTI10_15_IRQHandler              (void); //56:EXTI10 to EXTI15
void __attribute__((weak, alias ("Null_Handler"))) RTC_Alarm_IRQHandler              (void); //57:RTC Alarm
void __attribute__((weak, alias ("Null_Handler"))) USBFS_WKUP_IRQHandler             (void); //58:USBFS Wakeup
void __attribute__((weak, alias ("Null_Handler"))) TIMER7_BRK_TIMER11_IRQHandler     (void); //59:TIMER7 Break and TIMER11
void __attribute__((weak, alias ("Null_Handler"))) TIMER7_UP_TIMER12_IRQHandler      (void); //60:TIMER7 Update and TIMER12
void __attribute__((weak, alias ("Null_Handler"))) TIMER7_TRG_CMT_TIMER13_IRQHandler (void); //61:TIMER7 Trigger and Commutation and TIMER13
void __attribute__((weak, alias ("Null_Handler"))) TIMER7_Channel_IRQHandler         (void); //62:TIMER7 Channel Capture Compare
void __attribute__((weak, alias ("Null_Handler"))) EXMC_IRQHandler                   (void); //64:EXMC
void __attribute__((weak, alias ("Null_Handler"))) TIMER4_IRQHandler                 (void); //66:TIMER4
void __attribute__((weak, alias ("Null_Handler"))) SPI2_IRQHandler                   (void); //67:SPI2
void __attribute__((weak, alias ("Null_Handler"))) UART3_IRQHandler                  (void); //68:UART3
void __attribute__((weak, alias ("Null_Handler"))) UART4_IRQHandler                  (void); //69:UART4
void __attribute__((weak, alias ("Null_Handler"))) TIMER5_IRQHandler                 (void); //70:TIMER5 
void __attribute__((weak, alias ("Null_Handler"))) TIMER6_IRQHandler                 (void); //71:TIMER6
void __attribute__((weak, alias ("Null_Handler"))) DMA1_Channel0_IRQHandler          (void); //72:DMA1 Channel0
void __attribute__((weak, alias ("Null_Handler"))) DMA1_Channel1_IRQHandler          (void); //73:DMA1 Channel1
void __attribute__((weak, alias ("Null_Handler"))) DMA1_Channel2_IRQHandler          (void); //74:DMA1 Channel2
void __attribute__((weak, alias ("Null_Handler"))) DMA1_Channel3_IRQHandler          (void); //75:DMA1 Channel3
void __attribute__((weak, alias ("Null_Handler"))) DMA1_Channel4_IRQHandler          (void); //76:DMA1 Channel4
void __attribute__((weak, alias ("Null_Handler"))) USBFS_IRQHandler                  (void); //77:USBFS

//Interrupt vector table
const __attribute__((section(".vectors"))) uint32_t _VECTORS[]=
{
	(uint32_t)Reset_Handler                     , //Reset Handler
	(uint32_t)NMI_Handler                       , //NMI Handler
	(uint32_t)HardFault_Handler                 , //Hard Fault Handler
	(uint32_t)MemManage_Handler                 , //MPU Fault Handler
	(uint32_t)BusFault_Handler                  , //Bus Fault Handler
	(uint32_t)UsageFault_Handler                , //Usage Fault Handler
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)SVC_Handler                       , //SVCall Handler
	(uint32_t)DebugMon_Handler                  , //Debug Monitor Handler
	(uint32_t)0                                 , //Reserved
	(uint32_t)PendSV_Handler                    , //PendSV Handler
	(uint32_t)SysTick_Handler                   , //SysTick Handler

	//              /* external interrupts handler */
	(uint32_t)WWDGT_IRQHandler                  , //16:Window Watchdog Timer
	(uint32_t)LVD_IRQHandler                    , //17:LVD through EXTI Line detect
	(uint32_t)TAMPER_IRQHandler                 , //18:Tamper through EXTI Line detect
	(uint32_t)RTC_IRQHandler                    , //19:RTC through EXTI Line
	(uint32_t)FMC_IRQHandler                    , //20:FMC
	(uint32_t)RCU_CTC_IRQHandler                , //21:RCU and CTC
	(uint32_t)EXTI0_IRQHandler                  , //22:EXTI Line 0
	(uint32_t)EXTI1_IRQHandler                  , //23:EXTI Line 1
	(uint32_t)EXTI2_IRQHandler                  , //24:EXTI Line 2
	(uint32_t)EXTI3_IRQHandler                  , //25:EXTI Line 3
	(uint32_t)EXTI4_IRQHandler                  , //26:EXTI Line 4
	(uint32_t)DMA0_Channel0_IRQHandler          , //27:DMA0 Channel0
	(uint32_t)DMA0_Channel1_IRQHandler          , //28:DMA0 Channel1
	(uint32_t)DMA0_Channel2_IRQHandler          , //29:DMA0 Channel2
	(uint32_t)DMA0_Channel3_IRQHandler          , //30:DMA0 Channel3
	(uint32_t)DMA0_Channel4_IRQHandler          , //31:DMA0 Channel4
	(uint32_t)DMA0_Channel5_IRQHandler          , //32:DMA0 Channel5
	(uint32_t)DMA0_Channel6_IRQHandler          , //33:DMA0 Channel6
	(uint32_t)ADC0_1_IRQHandler                 , //34:ADC0 and ADC1
	(uint32_t)0                                 , //35:Reserved
	(uint32_t)0                                 , //36:Reserved
	(uint32_t)0                                 , //37:Reserved
	(uint32_t)0                                 , //38:Reserved
	(uint32_t)EXTI5_9_IRQHandler                , //39:EXTI5 to EXTI9
	(uint32_t)TIMER0_BRK_TIMER8_IRQHandler      , //40:TIMER0 Break and TIMER8
	(uint32_t)TIMER0_UP_TIMER9_IRQHandler       , //41:TIMER0 Update and TIMER9
	(uint32_t)TIMER0_TRG_CMT_TIMER10_IRQHandler , //42:TIMER0 Trigger and Commutation and TIMER10
	(uint32_t)TIMER0_Channel_IRQHandler         , //43:TIMER0 Channel Capture Compare
	(uint32_t)TIMER1_IRQHandler                 , //44:TIMER1
	(uint32_t)TIMER2_IRQHandler                 , //45:TIMER2
	(uint32_t)TIMER3_IRQHandler                 , //46:TIMER3
	(uint32_t)I2C0_EV_IRQHandler                , //47:I2C0 Event
	(uint32_t)I2C0_ER_IRQHandler                , //48:I2C0 Error
	(uint32_t)I2C1_EV_IRQHandler                , //49:I2C1 Event
	(uint32_t)I2C1_ER_IRQHandler                , //50:I2C1 Error
	(uint32_t)SPI0_IRQHandler                   , //51:SPI0
	(uint32_t)SPI1_IRQHandler                   , //52:SPI1
	(uint32_t)USART0_IRQHandler                 , //53:USART0
	(uint32_t)USART1_IRQHandler                 , //54:USART1
	(uint32_t)USART2_IRQHandler                 , //55:USART2
	(uint32_t)EXTI10_15_IRQHandler              , //56:EXTI10 to EXTI15
	(uint32_t)RTC_Alarm_IRQHandler              , //57:RTC Alarm
	(uint32_t)USBFS_WKUP_IRQHandler             , //58:USBFS Wakeup
	(uint32_t)TIMER7_BRK_TIMER11_IRQHandler     , //59:TIMER7 Break and TIMER11
	(uint32_t)TIMER7_UP_TIMER12_IRQHandler      , //60:TIMER7 Update and TIMER12
	(uint32_t)TIMER7_TRG_CMT_TIMER13_IRQHandler , //61:TIMER7 Trigger and Commutation and TIMER13
	(uint32_t)TIMER7_Channel_IRQHandler         , //62:TIMER7 Channel Capture Compare
	(uint32_t)0                                 , //63:Reserved
	(uint32_t)EXMC_IRQHandler                   , //64:EXMC
	(uint32_t)0                                 , //65:Reserved
	(uint32_t)TIMER4_IRQHandler                 , //66:TIMER4
	(uint32_t)SPI2_IRQHandler                   , //67:SPI2
	(uint32_t)UART3_IRQHandler                  , //68:UART3
	(uint32_t)UART4_IRQHandler                  , //69:UART4
	(uint32_t)TIMER5_IRQHandler                 , //70:TIMER5 
	(uint32_t)TIMER6_IRQHandler                 , //71:TIMER6
	(uint32_t)DMA1_Channel0_IRQHandler          , //72:DMA1 Channel0
	(uint32_t)DMA1_Channel1_IRQHandler          , //73:DMA1 Channel1
	(uint32_t)DMA1_Channel2_IRQHandler          , //74:DMA1 Channel2
	(uint32_t)DMA1_Channel3_IRQHandler          , //75:DMA1 Channel3
	(uint32_t)DMA1_Channel4_IRQHandler          , //76:DMA1 Channel4
	(uint32_t)0                                 , //77:Reserved
	(uint32_t)0                                 , //78:Reserved
	(uint32_t)0                                 , //79:Reserved
	(uint32_t)0                                 , //80:Reserved
	(uint32_t)0                                 , //81:Reserved
	(uint32_t)0                                 , //82:Reserved
	(uint32_t)USBFS_IRQHandler                  , //83:USBFS
};
