/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2011  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This file implements the platform specific functions for ST-Link
 * on the STM8S discovery and STM32F103 Minimum System Development Board, also
 * known as bluepill.
 */

#include "general.h"
#include "cdcacm.h"
#include "usbuart.h"

#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/stm32/f1/adc.h>

int platform_hwversion(void)
{
	return 1;
}

void platform_init(void)
{
	uint32_t data;
	SCS_DEMCR |= SCS_DEMCR_VC_MON_EN;
#ifdef ENABLE_DEBUG
	void initialise_monitor_handles(void);
	initialise_monitor_handles();
#endif

	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	/* Enable peripherals */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);

	rcc_periph_clock_enable(RCC_USB);
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_CRC);

	// LED pin setup
	gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_IDLE_RUN);
	gpio_set(LED_PORT, LED_IDLE_RUN);
	// USB output pin setup and reset
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_OPENDRAIN, GPIO12);
	gpio_clear(GPIOA, GPIO12);
	rcc_periph_reset_pulse(RST_USB);
	rcc_periph_clock_enable(RCC_USB);

	/* Unmap JTAG Pins so we can reuse as GPIO */	
	gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, 0);

	/* Setup JTAG GPIO ports */
	gpio_set_mode(TMS_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_INPUT_FLOAT, TMS_PIN);
	gpio_set_mode(TCK_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, TCK_PIN);
	gpio_set_mode(TDI_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, TDI_PIN);
	gpio_set_mode(TDO_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, TDO_PIN);

	/* Enable MCO Out on PA8*/
	RCC_CFGR &= ~(0xf << 24);
	RCC_CFGR |= (RCC_CFGR_MCO_HSE << 24);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO8);

	platform_srst_set_val(false);

	/* Remap TIM2 TIM2_REMAP[1]
	 * TIM2_CH1_ETR -> PA15 (TDI, set as output above)
	 * TIM2_CH2     -> PB3  (TDO)
	 */
	data = AFIO_MAPR;
	data &= ~AFIO_MAPR_TIM2_REMAP_FULL_REMAP;
	data |=  AFIO_MAPR_TIM2_REMAP_PARTIAL_REMAP1;
	AFIO_MAPR = data;

	/* Relocate interrupt vector table here */
	extern int vector_table;
	SCB_VTOR = (uint32_t)&vector_table;

	platform_timing_init();
	cdcacm_init();
	usbuart_init();
}

void platform_srst_set_val(bool assert)
{
	/* We reuse JSRST as SRST.*/
	if (assert) {
		gpio_set_mode(JRST_PORT, GPIO_MODE_OUTPUT_50_MHZ,
		              GPIO_CNF_OUTPUT_OPENDRAIN, JRST_PIN);
		/* Wait until requested value is active.*/
		while (gpio_get(JRST_PORT, JRST_PIN))
			gpio_clear(JRST_PORT, JRST_PIN);
	} else {
		gpio_set_mode(JRST_PORT, GPIO_MODE_INPUT,
					  GPIO_CNF_INPUT_PULL_UPDOWN, JRST_PIN);
		/* Wait until requested value is active.*/
		while (!gpio_get(JRST_PORT, JRST_PIN))
			gpio_set(JRST_PORT, JRST_PIN);
	}
}

bool platform_srst_get_val(void)
{
	return gpio_get(JRST_PORT, JRST_PIN) == 0;
}

const char *platform_target_voltage(void)
{
	return "Not supported";
}

void set_idle_state(int state)
{
	gpio_set_val(LED_PORT, LED_IDLE_RUN, state);
}
