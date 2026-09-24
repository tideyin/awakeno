// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef IR_SND_RCV_H
#define IR_SND_RCV_H


// For IR emulation Start
// For IR emulation , USE timer3 (NEC frame timing)
#define TIM3_PRESCALER 119
#define TIM3_1MS   1000   // 1ms
#define START_STEP1   9000   // 9ms
#define START_STEP2   4500  // 4.5ms
#define DATA_IR_EXIST 560 // 560us
#define DATA_ONE_NOIR 1690 // 2.25ms-560us=1.69ms
#define DATA_ZERO_NOIR 560 // 1.12ms- 560us=560us
#define DATA_41MS_NOIR 41000 //41ms
#define END_STEP1   9000   // 9ms
#define END_STEP2   2200  //  2.20ms
#define END_IR_EXIST 600 // 600us



//define common IR code
#define IR_CID                0x16  // 22
#define IR_CID_BAR            0x38  // 56

#define NEC_PWR  0x46
#define NEC_UP   0x48
#define NEC_DWN  0x4D
#define NEC_LFT  0x4E
#define NEC_RIT  0x49
#define NEC_VUP  0x0C
#define NEC_VDN  0x19
#define NEC_HOM  0x9F
#define NEC_BAK  0x0D
#define NEC_MUT  0x4C
#define NEC_MNU  0x45
#define NEC_PUP  0x0F
#define NEC_PDN  0x5A
#define NEC_SEL  0x4A
#define NEC_FWD  0x17
#define NEC_RWD  0x16
#define NEC_VOC  0xA0
#define NEC_PLY  0x5B


//For monitor
#define TIM5_PRESCALER 119
#define TIM5_1US   0xFFFF   // 1us


typedef enum {
    FRM_STEP1 = 0 ,
    FRM_STEP2 ,
    CID_STATE ,
    CID_BAR_STATE ,
    CODE_STATE ,
    CODE_BAR_STATE ,
    FRM2_41MS_STATE ,
    FRM2_STEP1 ,
    FRM2_STEP2 ,
    FRM2_STEP3 ,
    IR_STATE_MAX
} ir_state_t;

// For IR emulation
void ir_emu_init(void) ;
void send_ir_nec(uint8_t ir_code);
void TIM3_int_init(int prescaler, int period);
void ir_gpio_config(uint8_t ir_io_mode) ;
void ir_frq_timer_config(void) ;

// Above For IR emulation




// For IR receive and decode is start here
#define IR_MON_IO_CLOCK RCU_GPIOC



/* Awakeno: IR_RCV = PC9 → EXTI_9 (EXTI5_9) */
#define  IR_MON_EXIT_IRQn      EXTI5_9_IRQn
#define  IR_MON_EXIT           EXTI_9
#define  IR_MON_PORT_SOURCE    GPIO_PORT_SOURCE_GPIOC
#define  IR_MON_PIN_SOURCE     GPIO_PIN_SOURCE_9





//void ir_rcv_gpio_init(void);
//void ir_io_int_init(void);

void ir_mon_gpio_init(void);
void ir_mon_int_init(void);
void TIM5_counter_init(int prescaler, int period);

/* called from EXTI5_9_IRQHandler in matrix_key.c when EXTI_9 pending */
void ir_mon_stop_exti(void);
uint8_t ir_mon_is_active(void);
void ir_mon_exti9_isr(void);

#endif  /* IR_SND_RCV_H */
