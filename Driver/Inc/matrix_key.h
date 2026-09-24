// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef MATRIX_KEY_H
#define MATRIX_KEY_H

/*******************************************************************************
 * 4x4 matrix keypad (Awakeno)
 *
 * Hardware (PCB / silk): ROW on PB, COL on PC
 * Idle:
 *   ROW0..3 = PB8/PB9/PB12/PB13  — IPU + falling-edge EXTI
 *   COL0..3 = PC0..PC3           — push-pull output, drive 0
 *
 * On ROW fall: reverse scan — that ROW = OUT 0, COLs = IPU;
 *   COL that reads 0 is the pressed column → DAC tone
 *******************************************************************************/

#include "gd32e10x.h"

#define MATRIX_ROW_PORT     GPIOB
#define MATRIX_COL_PORT     GPIOC

#define MATRIX_ROW0_PIN     GPIO_PIN_8
#define MATRIX_ROW1_PIN     GPIO_PIN_9
#define MATRIX_ROW2_PIN     GPIO_PIN_12
#define MATRIX_ROW3_PIN     GPIO_PIN_13

#define MATRIX_COL0_PIN     GPIO_PIN_0
#define MATRIX_COL1_PIN     GPIO_PIN_1
#define MATRIX_COL2_PIN     GPIO_PIN_2
#define MATRIX_COL3_PIN     GPIO_PIN_3

#define MATRIX_ROW_NUM      4u
#define MATRIX_COL_NUM      4u

void matrix_key_init(void);

/* main-loop: after ROW EXTI, scan COL; play DAC tone */
void matrix_key_process(void);

#endif /* MATRIX_KEY_H */
