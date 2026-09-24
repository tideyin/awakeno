// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "gd32e10x.h"
#include "systick.h"
#include "ir_snd_rcv.h"
#include "rlk_gpio.h"


// for IR code emulation
extern uint8_t ir_cid , ir_cid_bar ; 
extern uint8_t ir_code_org , ir_code_bar ;
extern uint8_t pcba_flg ;

//for IR mon
extern uint16_t ir_counter ;	
extern uint8_t ir_mon_int_flag , timer5_int_flag;

static uint8_t ir_mon_active = 0;





/* Awakeno IR TX:
 * - TIMER7_CH2 @ PC8: 38kHz carrier (HW: IR_SND)
 * - TIMER3: NEC frame timing (same as WuKongFB; drv_timer moved to TIMER4)
 * - TIMER5 + EXTI5_9(PC9): IR monitor (optional RX path)
 */

void ir_emu_init(void)
{

	// for IR customer data INIT
	ir_cid = IR_CID ;
	ir_cid_bar = IR_CID_BAR ;
	pcba_flg = 0 ;

	// config IR GPIO mode	as GPIO_MODE_AF_PP(38K) or GPIO_MODE_OUT_PP (low)
	rcu_periph_clock_enable(RCU_AF);
	rcu_periph_clock_enable(RCU_TIMER7); // Must enable TIMER7 clock for 38k PWM	
	gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
	rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 1); // SET HIGHT
//	gpio_pin_remap_config(GPIO_TIMER7_PARTIAL_REMAP0, ENABLE); 

    ir_frq_timer_config() ; // init 38k timer
    timer_enable(TIMER7); 

}
void send_ir_nec(uint8_t ir_code)
{
    timer_interrupt_flag_clear(TIMER3, TIMER_INT_UP);
//	TIM3_int_init(11999, 499) ; //50ms ?
	gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
	rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 1); // Setup IO_EMU high

    // V38 init and enable TIMER7 every time  
	//ir_frq_timer_config() ;
	//timer_enable(TIMER7);

    TIM3_int_init(TIM3_PRESCALER, TIM3_1MS) ; 
	ir_code_org = ir_code ;
	ir_code_bar =  (~ir_code & 0xFF);

}


// ========= Timer3  ============

/* prescaler ????????? 120MHz / (prescaler + 1)
 * ?????? T = (1s / (120MHz / (prescaler + 1))) * (period + 1)
 */
// TIM3_int_init(11999, 499); // 50ms 
// TIM3_int_init(119, 1000); // 1ms 
// TIM3_int_init(119, 1); // 1us 
// TIM2_int_init(119, 49999); // 50ms 
// TIM2_int_init(119, 10000); // 10ms
// TIM2_int_init(119, 9000); // 10ms

void TIM3_int_init(int prescaler, int period)
{
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER3);

    // V38 init and enable TIMER7 every time  
	ir_frq_timer_config() ;
	timer_enable(TIMER7);

    /* initialize TIMER init parameter struct */
    timer_struct_para_init(&timer_initpara);
    /* TIMER3 configuration */

    timer_initpara.prescaler         = prescaler;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = period;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER3, &timer_initpara);

    timer_interrupt_flag_clear(TIMER3, TIMER_INT_UP);  //??????????????????
    timer_interrupt_enable(TIMER3, TIMER_INT_UP);
    nvic_irq_enable(TIMER3_IRQn, 4, 1);//??????????

    /* enable */
    timer_enable(TIMER3);
}





void TIMER3_IRQHandler(void)
{
	static uint32_t ir_state = 0;
    static uint8_t data_seq_flg , bit  ;
	static uint8_t ir_data ;

	timer_enable(TIMER7);

    if(SET == timer_interrupt_flag_get(TIMER3, TIMER_INT_UP)) 
	{
        /* clear channel 0 interrupt bit */
        timer_interrupt_flag_clear(TIMER3, TIMER_INT_UP);
    // IR code generation
       switch (ir_state++)
	   {
		   case FRM_STEP1: 
		   {
		     
             if(pcba_flg == 1)		
             {
			   gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);   
               // 9ms "0"
			   rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN,0); // Setup IO_EMU LOW
             }	
		     else
		     {
				gpio_init(IR_EMU_IO_PORT, GPIO_MODE_AF_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
		     }	
			   	TIM3_int_init(TIM3_PRESCALER, START_STEP1) ; 
		   }
		   break;
		   
		   case FRM_STEP2: 
		   {
		   		gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
				rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 1); // Setup IO_EMU HIGH
				TIM3_int_init(TIM3_PRESCALER, START_STEP2) ; 
			    data_seq_flg = 1 ;
			    bit = 1;
			    ir_data = ir_cid ; //IR_CID ;  
		   }
		   break;
	   
		   case CID_STATE:
		   {
		  	   
			   if (data_seq_flg == 1)
			   {
				 data_seq_flg = 0 ;
			    if(pcba_flg == 1)		 
			    {
				  gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
			 	  rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 0); // Setup IO_EMU LOW 
			   	}	 
			    else
			   	{
			   	  gpio_init(IR_EMU_IO_PORT, GPIO_MODE_AF_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
			   	}
			   
				 ir_state-- ;
				 TIM3_int_init(TIM3_PRESCALER, DATA_IR_EXIST) ;	
#if 0				 
				 if(bit==0x0)
				 {	
				   ir_state++ ;
				   bit = 1;
				   ir_data = IR_CID_BAR ;  
				 }	 
#endif				 
			   }
			   else
			   {
				 data_seq_flg = 1 ;
				 gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
				 rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 1); // Setup IO_EMU HIGH				 

				 if(bit)
				 {	   
					 if (bit & ir_data )
					 {
					   TIM3_int_init(TIM3_PRESCALER, DATA_ONE_NOIR) ; 
					 }
					 else
					 {
					   TIM3_int_init(TIM3_PRESCALER, DATA_ZERO_NOIR) ; 
					 }
					 ir_state-- ;
					 bit <<=1;
				 }	  
#if 1				 
				 if(bit==0x0)
				 {	
				   ir_state++ ;
				   data_seq_flg = 1 ;
				   bit = 1;
				   ir_data = ir_cid_bar; // IR_CID_BAR ;  
				 }	 
#endif				 
			   } 

		   }
		   break;
		   
		   
		   case CID_BAR_STATE:
		   {
			   if (data_seq_flg == 1)
			   {
				 data_seq_flg = 0 ;
				 if(pcba_flg == 1)
				 {
				   gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
				   rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 0); // Setup IO_EMU LOW
				 }
				 else
				 {
                    gpio_init(IR_EMU_IO_PORT, GPIO_MODE_AF_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
				 }
				 ir_state-- ;
				 TIM3_int_init(TIM3_PRESCALER, DATA_IR_EXIST) ;	
#if 0				 
				 if(bit==0x0)
				 {	
				   ir_state++ ;
				   bit = 1;
				   ir_data = IR_CID_BAR ;  
				 }	 
#endif				 
			   }
			   else
			   {
				 data_seq_flg = 1 ;
				 gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
				 rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 1); // Setup IO_EMU HIGH				 

				 if(bit)
				 {	   
					 if (bit & ir_data )
					 {
					   TIM3_int_init(TIM3_PRESCALER, DATA_ONE_NOIR) ; 
					 }
					 else
					 {
					   TIM3_int_init(TIM3_PRESCALER, DATA_ZERO_NOIR) ; 
					 }
					 ir_state-- ;
					 bit <<=1;
				 }	  
				 
#if 1				 
								  if(bit==0x0)
								  {  
									ir_state++ ;
								    data_seq_flg = 1 ;
									bit = 1;
									ir_data = ir_code_org ;	
								  }   
#endif

			   } 
		   }
		   break;
		   
		   case CODE_STATE:
		   {
			   if (data_seq_flg == 1)
			   {
				 data_seq_flg = 0 ;
				 if(pcba_flg == 1)
				 {
				   gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
			  	   rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 0); // Setup IO_EMU LOW  
				 }
				 else
				 {
				     //38k
					 gpio_init(IR_EMU_IO_PORT, GPIO_MODE_AF_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
				 }
				 ir_state-- ;
				 TIM3_int_init(TIM3_PRESCALER, DATA_IR_EXIST) ;
#if 0				 
				 if(bit==0x0)
				 {	
				   ir_state++ ;
				   bit = 1;
				   ir_data = IR_CID_BAR ;  
				 }	 
#endif				 
			   }
			   else
			   {
				 data_seq_flg = 1 ;
				 gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
				 rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 1); // Setup IO_EMU HIGH				 

				 if(bit)
				 {	   
					 if (bit & ir_data )
					 {
					   TIM3_int_init(TIM3_PRESCALER, DATA_ONE_NOIR) ; 
					 }
					 else
					 {
					   TIM3_int_init(TIM3_PRESCALER, DATA_ZERO_NOIR) ; 
					 }
					 ir_state-- ;
					 bit <<=1;
				 }	  
#if 1				 
								  if(bit==0x0)
								  {  
									ir_state++ ;
								    data_seq_flg = 1 ;
									bit = 1;
									ir_data = ir_code_bar ;	
								  }   
#endif

			   }   
		   }
		   break;
		   
		   case CODE_BAR_STATE:
		   {
			   if (data_seq_flg == 1)
			   {
				 data_seq_flg = 0 ;
				 if(pcba_flg == 1)
				 {
					 gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
		  		     rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 0); // Setup IO_EMU LOW  
				 }
				 else
				 {
				    //38k
					gpio_init(IR_EMU_IO_PORT, GPIO_MODE_AF_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
				 }
				 ir_state-- ;
				 TIM3_int_init(TIM3_PRESCALER, DATA_IR_EXIST) ;
#if 1 // Last bit				 
				 if(bit==0x0)
				 {	
				   ir_state++ ;
				 }	 
#endif		 
			   }
			   else
			   {
				 data_seq_flg = 1 ;
				 gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
				 rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 1); // Setup IO_EMU HIGH				 

				 if(bit)
				 {	   
					 if (bit & ir_data )
					 {
					   TIM3_int_init(TIM3_PRESCALER, DATA_ONE_NOIR) ; 
					 }
					 else
					 {
					   TIM3_int_init(TIM3_PRESCALER, DATA_ZERO_NOIR) ; 
					 }
					 ir_state-- ;
					 bit <<=1;
				 }	  
#if 0				 
								  if(bit==0x0)
								  {  
									ir_state++ ;
								  }   
#endif

			   }    
		   }
		   break;  
		   case FRM2_41MS_STATE: 
		   {
			    gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
				rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 1); // Setup IO_EMU HIGH
			   	TIM3_int_init(TIM3_PRESCALER, DATA_41MS_NOIR) ; 
		   }
		   break;
		   case FRM2_STEP1: 
		   {
				 if(pcba_flg == 1)
				 {	   
					 gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
				     rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 0); // Setup IO_EMU LOW
				 }
				 else
				 {
				    gpio_init(IR_EMU_IO_PORT, GPIO_MODE_AF_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
				 }

				   TIM3_int_init(TIM3_PRESCALER, END_STEP1) ; 
		   }
		   break;
		   case FRM2_STEP2: 
		   {
			   gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
			   rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 1); // Setup IO_EMU HIGH
			   TIM3_int_init(TIM3_PRESCALER, END_STEP2) ; 
		   }
		   break;		   
		   case FRM2_STEP3: 
		   {
			   if(pcba_flg == 1)
			   {	   
				   gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
			       rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 0); // Setup IO_EMU LOW
			   }
			   else
			   {
		         gpio_init(IR_EMU_IO_PORT, GPIO_MODE_AF_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
			   } 	  
			   TIM3_int_init(TIM3_PRESCALER, END_IR_EXIST) ; 
		   }
		   break;	
		   
		   case IR_STATE_MAX:	
			 {
			     //Resume GPIO
			     gpio_init(IR_EMU_IO_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
				 ir_state = 0;
				 rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 1); // Setup IO_EMU HIGH				 
				 timer_interrupt_disable(TIMER3, TIMER_INT_UP);
				 nvic_irq_disable(TIMER3_IRQn); //disable TIMER3 interrupt		 
			 }
		   break;
		   
		   default:
			   break;
		   }
		   




    }
}




//====     
void ir_gpio_config(uint8_t ir_io_mode)
{
//    rcu_periph_clock_enable(RCU_GPIOA);
//    rcu_periph_clock_enable(RCU_AF);
// config IR GPIO mode	as GPIO_MODE_AF_PP(38K) or GPIO_MODE_OUT_PP (low)
    gpio_init(IR_EMU_IO_PORT, ir_io_mode , GPIO_OSPEED_50MHZ, IR_EMU_IO_PIN);
    rlk_gpio_set_value(IR_EMU_IO_PORT, IR_EMU_IO_PIN, 0^pcba_flg);
 //   gpio_bit_reset(IR_EMU_IO_PORT, IR_EMU_IO_PIN);
 
}

/**
    \brief      configure the TIMER peripheral
    \param[in]  none
    \param[out] none
    \retval     none
  */
// For 38K carrier: TIMER7_CH2 on PC8 (Awakeno IR_SND)
void ir_frq_timer_config(void)
{
    /* -----------------------------------------------------------------------
    TIMER2 configuration: generate 3 PWM signals with 3 different duty cycles:
    TIMER7CLK = SystemCoreClock / 120 = 1MHz, the PWM frequency is 62.5Hz.

    TIMER2 channel0 duty cycle = (4000/ 16000)* 100  = 25%
    TIMER2 channel1 duty cycle = (8000/ 16000)* 100  = 50%
    TIMER2 channel2 duty cycle = (12000/ 16000)* 100 = 75%
    ----------------------------------------------------------------------- */
    timer_oc_parameter_struct timer_ocinitpara;
    timer_parameter_struct timer_initpara;

//Should use Timer , Timer4 cannot be exported from PA1(GPIO)
    rcu_periph_clock_enable(RCU_TIMER7);

    timer_deinit(TIMER7);
    /* initialize TIMER init parameter struct */
    timer_struct_para_init(&timer_initpara);


    /* TIMER4 configuration */
#ifdef PWM_HIGH_SPEED
    timer_initpara.prescaler         = 2;
#else
    timer_initpara.prescaler         = 12;
#endif
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 244;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;



    timer_init(TIMER7, &timer_initpara);

    /* initialize TIMER channel output parameter struct */
    timer_channel_output_struct_para_init(&timer_ocinitpara);
    /* CH0, CH1 and CH2 configuration in PWM mode */
    timer_ocinitpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocinitpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocinitpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocinitpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocinitpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocinitpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER7, TIMER_CH_2, &timer_ocinitpara);

    /* CH2 PWM ~38kHz carrier on PC8 */
    timer_channel_output_pulse_value_config(TIMER7, TIMER_CH_2, 100);
    timer_channel_output_mode_config(TIMER7, TIMER_CH_2, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER7, TIMER_CH_2, TIMER_OC_SHADOW_DISABLE);

    /* TIMER7 is advanced timer: must enable primary output */
    timer_primary_output_config(TIMER7, ENABLE);

    /* TIMER7 only for 38k ,no interrupt */
    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER7);
}

// ================  For IR Monitor  ================
//Below is for IR receive and decode
void ir_mon_gpio_init(void)
{
    /* enable the ir rcv io GPIO clock */
    rcu_periph_clock_enable(IR_MON_IO_CLOCK);
    /* configure button pin as input */
    gpio_init(IR_MON_IO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, IR_MON_IO_PIN);
}


// init rcv io interupt 
void ir_mon_int_init()
{

    /* ??????pin???exti */
    gpio_exti_source_select(IR_MON_PORT_SOURCE, IR_MON_PIN_SOURCE);

    /* Setup trigger , Both edge    */
    exti_init(IR_MON_EXIT, EXTI_INTERRUPT, EXTI_TRIG_BOTH);

    /* ????????? */
    exti_interrupt_flag_clear(IR_MON_EXIT);


    /* disable */
    timer_disable(TIMER5);
// Timer5 1us conter for IR monitor
	TIM5_counter_init(TIM5_PRESCALER, TIM5_1US) ; 

	ir_mon_int_flag = 0 ;
	timer5_int_flag = 0 ;
	ir_counter = 0;
	ir_mon_active = 1;

    /* EXTI5_9 NVIC already enabled by matrix_key; only enable EXTI9 line */
    nvic_irq_enable(IR_MON_EXIT_IRQn, 2U, 0U);	
    timer_counter_value_config(TIMER5,0x0) ;

}

uint8_t ir_mon_is_active(void)
{
	return ir_mon_active;
}

void ir_mon_stop_exti(void)
{
	ir_mon_active = 0;
	exti_interrupt_disable(IR_MON_EXIT);
	/* give EXTI9 back to matrix ROW1 (PB9) */
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_9);
	exti_init(EXTI_9, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
	exti_interrupt_flag_clear(EXTI_9);
	exti_interrupt_enable(EXTI_9);
}


/* PC9 / EXTI_9 — invoked from matrix_key.c EXTI5_9_IRQHandler */
void ir_mon_exti9_isr(void)
{
	if (!ir_mon_active) {
		return;
	}
	if (SET == exti_interrupt_flag_get(IR_MON_EXIT)) {
		timer_disable(TIMER5);
		ir_counter = timer_counter_read(TIMER5);
		timer_counter_value_config(TIMER5, 0x0);
		timer_enable(TIMER5);
		exti_interrupt_flag_clear(IR_MON_EXIT);
		ir_mon_int_flag = 1;
		exti_interrupt_disable(IR_MON_EXIT);
		nvic_irq_enable(TIMER5_IRQn, 4, 1);
	}
}

//Above is for IR Monitor

void TIM5_counter_init(int prescaler, int period)
{
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER5);

    /* initialize TIMER init parameter struct */
    timer_struct_para_init(&timer_initpara);
    /* TIMER3 configuration */

    timer_initpara.prescaler         = prescaler;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = period;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER5, &timer_initpara);

    timer_interrupt_flag_clear(TIMER5, TIMER_INT_UP);  //??????????????????
    timer_interrupt_enable(TIMER5, TIMER_INT_UP);
 //   nvic_irq_enable(TIMER5_IRQn, 4, 1);//??????????	


}


// Timer5 tick is 1us , overflow is 65535us = 65.535ms
void TIMER5_IRQHandler(void)
{

     if(SET == timer_interrupt_flag_get(TIMER5, TIMER_INT_UP)) 
     {
		 /* clear channel 0 interrupt bit */
		timer_interrupt_flag_clear(TIMER5, TIMER_INT_UP);
	    timer_disable(TIMER5);
	    ir_counter = 0;  // Timer5 counter overflow 0xffff or 0 ,set to 0
	    timer_counter_value_config(TIMER5,0) ; 
		timer5_int_flag = 1;		 
     }

//	 nvic_irq_enable(TIMER5_IRQn, 4, 1);//?????????? 
     nvic_irq_disable(TIMER5_IRQn) ; // Disable Timer5 , Enalbe should be in  IR_MON_EXIT_IRQn
}






