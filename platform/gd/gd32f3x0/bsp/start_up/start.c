/*
Startup file for GD32F3x0
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
void __attribute__((weak, alias ("Null_Handler"))) RTC_IRQHandler                    (void); //18:RTC through EXTI Line
void __attribute__((weak, alias ("Null_Handler"))) FMC_IRQHandler                    (void); //19:FMC
void __attribute__((weak, alias ("Null_Handler"))) RCU_CTC_IRQHandler                (void); //20:RCU and CTC
void __attribute__((weak, alias ("Null_Handler"))) EXTI0_1_IRQHandler                (void); //21:EXTI Line 0 and EXTI Line 1
void __attribute__((weak, alias ("Null_Handler"))) EXTI2_3_IRQHandler                (void); //22:EXTI Line 2 and EXTI Line 3
void __attribute__((weak, alias ("Null_Handler"))) EXTI4_15_IRQHandler               (void); //23:EXTI Line 4 to EXTI Line 15
void __attribute__((weak, alias ("Null_Handler"))) TSI_IRQHandler                    (void); //24:TSI
void __attribute__((weak, alias ("Null_Handler"))) DMA_Channel0_IRQHandler           (void); //25:DMA Channel 0 
void __attribute__((weak, alias ("Null_Handler"))) DMA_Channel1_2_IRQHandler         (void); //26:DMA Channel 1 and DMA Channel 2
void __attribute__((weak, alias ("Null_Handler"))) DMA_Channel3_4_IRQHandler         (void); //27:DMA Channel 3 and DMA Channel 4
void __attribute__((weak, alias ("Null_Handler"))) ADC_CMP_IRQHandler                (void); //28:ADC and Comparator 0-1
void __attribute__((weak, alias ("Null_Handler"))) TIMER0_BRK_UP_TRG_COM_IRQHandler  (void); //29:TIMER0 Break,Update,Trigger and Commutation
void __attribute__((weak, alias ("Null_Handler"))) TIMER0_Channel_IRQHandler         (void); //30:TIMER0 Channel Capture Compare
void __attribute__((weak, alias ("Null_Handler"))) TIMER1_IRQHandler                 (void); //31:TIMER1
void __attribute__((weak, alias ("Null_Handler"))) TIMER2_IRQHandler                 (void); //32:TIMER2
void __attribute__((weak, alias ("Null_Handler"))) TIMER5_DAC_IRQHandler             (void); //33:TIMER5 and DAC
void __attribute__((weak, alias ("Null_Handler"))) TIMER13_IRQHandler                (void); //35:TIMER13
void __attribute__((weak, alias ("Null_Handler"))) TIMER14_IRQHandler                (void); //36:TIMER14
void __attribute__((weak, alias ("Null_Handler"))) TIMER15_IRQHandler                (void); //37:TIMER15
void __attribute__((weak, alias ("Null_Handler"))) TIMER16_IRQHandler                (void); //38:TIMER16
void __attribute__((weak, alias ("Null_Handler"))) I2C0_EV_IRQHandler                (void); //39:I2C0 Event
void __attribute__((weak, alias ("Null_Handler"))) I2C1_EV_IRQHandler                (void); //40:I2C1 Event
void __attribute__((weak, alias ("Null_Handler"))) SPI0_IRQHandler                   (void); //41:SPI0
void __attribute__((weak, alias ("Null_Handler"))) SPI1_IRQHandler                   (void); //42:SPI1
void __attribute__((weak, alias ("Null_Handler"))) USART0_IRQHandler                 (void); //43:USART0
void __attribute__((weak, alias ("Null_Handler"))) USART1_IRQHandler                 (void); //44:USART1
void __attribute__((weak, alias ("Null_Handler"))) CEC_IRQHandler                    (void); //46:CEC
void __attribute__((weak, alias ("Null_Handler"))) I2C0_ER_IRQHandler                (void); //48:I2C0 Error
void __attribute__((weak, alias ("Null_Handler"))) I2C1_ER_IRQHandler                (void); //50:I2C1 Error
void __attribute__((weak, alias ("Null_Handler"))) USBFS_WKUP_IRQHandler             (void); //58:USBFS Wakeup
void __attribute__((weak, alias ("Null_Handler"))) DMA_Channel5_6_IRQHandler         (void); //64:DMA Channel5 and Channel6 
void __attribute__((weak, alias ("Null_Handler"))) USBFS_IRQHandler                  (void); //83:USBFS

//Interrupt vector table
const __attribute__((section(".vectors"))) uint32_t _VECTORS[]=
{
    (uint32_t)WWDGT_IRQHandler                  , //16:Window Watchdog Timer
	(uint32_t)LVD_IRQHandler                    , //17:LVD through EXTI Line detect
	(uint32_t)RTC_IRQHandler                    , //18:RTC through EXTI Line
	(uint32_t)FMC_IRQHandler                    , //19:FMC
	(uint32_t)RCU_CTC_IRQHandler                , //20:RCU and CTC
	(uint32_t)EXTI0_1_IRQHandler                , //21:EXTI Line 0 and EXTI Line 1
	(uint32_t)EXTI2_3_IRQHandler                , //22:EXTI Line 2 and EXTI Line 3
	(uint32_t)EXTI4_15_IRQHandler               , //23:EXTI Line 4 to EXTI Line 15
	(uint32_t)TSI_IRQHandler                    , //24:TSI
	(uint32_t)DMA_Channel0_IRQHandler           , //25:DMA Channel 0 
	(uint32_t)DMA_Channel1_2_IRQHandler         , //26:DMA Channel 1 and DMA Channel 2
	(uint32_t)DMA_Channel3_4_IRQHandler         , //27:DMA Channel 3 and DMA Channel 4
	(uint32_t)ADC_CMP_IRQHandler                , //28:ADC and Comparator 0-1
	(uint32_t)TIMER0_BRK_UP_TRG_COM_IRQHandler  , //29:TIMER0 Break,Update,Trigger and Commutation
	(uint32_t)TIMER0_Channel_IRQHandler         , //30:TIMER0 Channel Capture Compare
	(uint32_t)TIMER1_IRQHandler                 , //31:TIMER1
	(uint32_t)TIMER2_IRQHandler                 , //32:TIMER2
	(uint32_t)TIMER5_DAC_IRQHandler             , //33:TIMER5 and DAC
	(uint32_t)0                                 , //Reserved
	(uint32_t)TIMER13_IRQHandler                , //35:TIMER13
	(uint32_t)TIMER14_IRQHandler                , //36:TIMER14
	(uint32_t)TIMER15_IRQHandler                , //37:TIMER15
	(uint32_t)TIMER16_IRQHandler                , //38:TIMER16
	(uint32_t)I2C0_EV_IRQHandler                , //39:I2C0 Event
	(uint32_t)I2C1_EV_IRQHandler                , //40:I2C1 Event
	(uint32_t)SPI0_IRQHandler                   , //41:SPI0
	(uint32_t)SPI1_IRQHandler                   , //42:SPI1
	(uint32_t)USART0_IRQHandler                 , //43:USART0
	(uint32_t)USART1_IRQHandler                 , //44:USART1
	(uint32_t)0                                 , //Reserved
	(uint32_t)CEC_IRQHandler                    , //46:CEC
	(uint32_t)0                                 , //Reserved
	(uint32_t)I2C0_ER_IRQHandler                , //48:I2C0 Error
	(uint32_t)0                                 , //Reserved
	(uint32_t)I2C1_ER_IRQHandler                , //50:I2C1 Error
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)USBFS_WKUP_IRQHandler             , //58:USBFS Wakeup
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)DMA_Channel5_6_IRQHandler         , //64:DMA Channel5 and Channel6 
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)0                                 , //Reserved
	(uint32_t)USBFS_IRQHandler                  , //83:USBFS
};
