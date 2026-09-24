// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "matrix_key.h"
#include "systick.h"
#include "ir_snd_rcv.h"
#include "tone.h"
#include "printf.h"

static const uint32_t row_pins[MATRIX_ROW_NUM] = {
	MATRIX_ROW0_PIN, MATRIX_ROW1_PIN, MATRIX_ROW2_PIN, MATRIX_ROW3_PIN
};
static const uint32_t col_pins[MATRIX_COL_NUM] = {
	MATRIX_COL0_PIN, MATRIX_COL1_PIN, MATRIX_COL2_PIN, MATRIX_COL3_PIN
};
static const uint8_t row_pin_src[MATRIX_ROW_NUM] = {
	GPIO_PIN_SOURCE_8, GPIO_PIN_SOURCE_9, GPIO_PIN_SOURCE_12, GPIO_PIN_SOURCE_13
};
static const exti_line_enum row_exti[MATRIX_ROW_NUM] = {
	EXTI_8, EXTI_9, EXTI_12, EXTI_13
};

static volatile uint8_t mtx_irq_pending = 0;
static volatile uint8_t mtx_irq_row = 0xFF;

static void matrix_cols_out_low(void)
{
	uint32_t i;

	for (i = 0; i < MATRIX_COL_NUM; i++) {
		gpio_init(MATRIX_COL_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, col_pins[i]);
		gpio_bit_reset(MATRIX_COL_PORT, col_pins[i]);
	}
}

static void matrix_rows_ipu_exti(void)
{
	uint32_t i;

	for (i = 0; i < MATRIX_ROW_NUM; i++) {
		gpio_init(MATRIX_ROW_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, row_pins[i]);
		gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, row_pin_src[i]);
		exti_init(row_exti[i], EXTI_INTERRUPT, EXTI_TRIG_FALLING);
		exti_interrupt_flag_clear(row_exti[i]);
		exti_interrupt_enable(row_exti[i]);
	}
}

static void matrix_idle_config(void)
{
	matrix_cols_out_low();
	matrix_rows_ipu_exti();
}

static void matrix_row_isr(uint8_t row)
{
	if (row >= MATRIX_ROW_NUM) {
		return;
	}
	if (!mtx_irq_pending) {
		mtx_irq_pending = 1;
		mtx_irq_row = row;
	}
	exti_interrupt_flag_clear(row_exti[row]);
	exti_interrupt_disable(row_exti[row]);
}

void EXTI5_9_IRQHandler(void)
{
	if (SET == exti_interrupt_flag_get(EXTI_8)) {
		matrix_row_isr(0);
	}
	if (SET == exti_interrupt_flag_get(EXTI_9)) {
		if (ir_mon_is_active()) {
			ir_mon_exti9_isr();
		} else {
			matrix_row_isr(1);
		}
	}
}

void EXTI10_15_IRQHandler(void)
{
	if (SET == exti_interrupt_flag_get(EXTI_12)) {
		matrix_row_isr(2);
	}
	if (SET == exti_interrupt_flag_get(EXTI_13)) {
		matrix_row_isr(3);
	}
}

/* reverse: drive pressed ROW = OUT 0; COLs = IPU; find COL reading 0 */
static int matrix_scan_col(uint8_t row, uint8_t *col_out)
{
	uint32_t i;
	uint8_t found = 0xFF;

	if (row >= MATRIX_ROW_NUM) {
		return -1;
	}

	for (i = 0; i < MATRIX_COL_NUM; i++) {
		gpio_init(MATRIX_COL_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, col_pins[i]);
	}
	for (i = 0; i < MATRIX_ROW_NUM; i++) {
		if (i == row) {
			gpio_init(MATRIX_ROW_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, row_pins[i]);
			gpio_bit_reset(MATRIX_ROW_PORT, row_pins[i]);
		} else {
			gpio_init(MATRIX_ROW_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, row_pins[i]);
		}
	}

	delay_1ms(1);

	for (i = 0; i < MATRIX_COL_NUM; i++) {
		if (RESET == gpio_input_bit_get(MATRIX_COL_PORT, col_pins[i])) {
			found = (uint8_t)i;
			break;
		}
	}

	matrix_idle_config();

	if (found >= MATRIX_COL_NUM) {
		return -1;
	}
	*col_out = found;
	return 0;
}

void matrix_key_init(void)
{
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_AF);

	mtx_irq_pending = 0;
	mtx_irq_row = 0xFF;

	matrix_idle_config();

	nvic_irq_enable(EXTI5_9_IRQn, 2U, 0U);
	nvic_irq_enable(EXTI10_15_IRQn, 2U, 0U);

	printf_("\r\nMatrix key init (ROW PB8/9/12/13 EXTI, COL PC0-3 out0)");
}

void matrix_key_process(void)
{
	uint8_t row;
	uint8_t col = 0;
	uint32_t wait;

	if (!mtx_irq_pending) {
		return;
	}

	row = mtx_irq_row;
	mtx_irq_pending = 0;

	/* busy-wait debounce — safe while I2S TBE IRQ may be running */
	delay_while_ms(10);

	if (0 != matrix_scan_col(row, &col)) {
		exti_interrupt_flag_clear(row_exti[row]);
		exti_interrupt_enable(row_exti[row]);
		return;
	}

	tone_play_key(row, col);
	printf_("\r\nMatrix key ROW=%u,COL=%u", (unsigned)row, (unsigned)col);

	/* wait for release before EXTI on — held key would immediately re-fire */
	matrix_cols_out_low();
	for (wait = 0u; wait < 500u; wait++) {
		if (SET == gpio_input_bit_get(MATRIX_ROW_PORT, row_pins[row])) {
			break;
		}
		delay_while_ms(1);
	}
	delay_while_ms(10);

	exti_interrupt_flag_clear(row_exti[row]);
	exti_interrupt_enable(row_exti[row]);
}
