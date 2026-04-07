/****************************************************************************
 *
 *   Copyright (c) 2024 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file board_config.h
 *
 * HAKRC F405 V2 internal definitions.
 *
 * MCU: STM32F405RGT6 (same as Omnibus F4 SD)
 *
 * Motor outputs (DShot-capable via TIM3 / TIM2 DMA):
 *   M1  TIM3_CH3  PB0   front (CCW)
 *   M2  TIM3_CH4  PB1   right (CW)
 *   M3  TIM2_CH4  PA3   rear  (CCW)
 *   M4  TIM2_CH3  PA2   left  (CW)
 *
 * Servo outputs (standard 50 Hz PWM via TIM8):
 *   S1  TIM8_CH3  PC8
 *   S2  TIM8_CH4  PC9
 *   S3  TIM8_CH1  PC6   (shared with UART6 TX — mutually exclusive)
 *   S4  TIM8_CH2  PC7   (shared with UART6 RX — mutually exclusive)
 *
 * SPI buses:
 *   SPI1  IMU MPU6000   CS=PA4
 *   SPI2  SD card       CS=PB12
 *   SPI3  BMP280        CS=PB3
 *         MAX7456 OSD   CS=PA15
 *
 * I2C1  External compass  PB6(SCL)/PB7(SDA)
 *
 * UARTs:
 *   USART1  GPS        PA9 TX / PA10 RX
 *   USART3  Telemetry  PB10 TX / PB11 RX
 *   USART6  RC serial  PC6 TX / PC7 RX  (shared with S3/S4)
 *
 * ADC:
 *   ADC1_IN12  PC2  battery current
 *   ADC1_IN13  PC3  battery voltage
 *
 * LED: PB5 (blue, active low)
 */

#pragma once

#include <px4_platform_common/px4_config.h>
#include <nuttx/compiler.h>
#include <stdint.h>

/* LED — blue on PB5, active low */
#define GPIO_LED1      (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTB|GPIO_PIN5)
#define GPIO_LED_BLUE  GPIO_LED1

#define BOARD_OVERLOAD_LED  LED_BLUE

#define FLASH_BASED_PARAMS

/* ADC channels */
#define ADC_CHANNELS                  ((1 << 12) | (1 << 13))
#define ADC_BATTERY_CURRENT_CHANNEL   12   /* PC2 — ADC1_IN12 */
#define ADC_BATTERY_VOLTAGE_CHANNEL   13   /* PC3 — ADC1_IN13 */

/* GPIO helpers */
#define _MK_GPIO_INPUT(def)  (((def) & (GPIO_PORT_MASK | GPIO_PIN_MASK)) | (GPIO_INPUT|GPIO_PULLUP))
#define _MK_GPIO_OUTPUT(def) (((def) & (GPIO_PORT_MASK | GPIO_PIN_MASK)) | (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR))

/* Input mode definitions (used by io_timer when reconfiguring) */
#define GPIO_GPIO0_INPUT   _MK_GPIO_INPUT(GPIO_TIM3_CH3IN)
#define GPIO_GPIO1_INPUT   _MK_GPIO_INPUT(GPIO_TIM3_CH4IN)
#define GPIO_GPIO2_INPUT   _MK_GPIO_INPUT(GPIO_TIM2_CH4IN)
#define GPIO_GPIO3_INPUT   _MK_GPIO_INPUT(GPIO_TIM2_CH3IN)
#define GPIO_GPIO4_INPUT   _MK_GPIO_INPUT(GPIO_TIM8_CH3IN)
#define GPIO_GPIO5_INPUT   _MK_GPIO_INPUT(GPIO_TIM8_CH4IN)
#define GPIO_GPIO6_INPUT   _MK_GPIO_INPUT(GPIO_TIM8_CH1IN)
#define GPIO_GPIO7_INPUT   _MK_GPIO_INPUT(GPIO_TIM8_CH2IN)

/* Output mode definitions */
#define GPIO_GPIO0_OUTPUT  _MK_GPIO_OUTPUT(GPIO_TIM3_CH3OUT)
#define GPIO_GPIO1_OUTPUT  _MK_GPIO_OUTPUT(GPIO_TIM3_CH4OUT)
#define GPIO_GPIO2_OUTPUT  _MK_GPIO_OUTPUT(GPIO_TIM2_CH4OUT)
#define GPIO_GPIO3_OUTPUT  _MK_GPIO_OUTPUT(GPIO_TIM2_CH3OUT)
#define GPIO_GPIO4_OUTPUT  _MK_GPIO_OUTPUT(GPIO_TIM8_CH3OUT)
#define GPIO_GPIO5_OUTPUT  _MK_GPIO_OUTPUT(GPIO_TIM8_CH4OUT)
#define GPIO_GPIO6_OUTPUT  _MK_GPIO_OUTPUT(GPIO_TIM8_CH1OUT)
#define GPIO_GPIO7_OUTPUT  _MK_GPIO_OUTPUT(GPIO_TIM8_CH2OUT)

/* USB OTG FS VBUS sensing — PA9 */
#define GPIO_OTGFS_VBUS  (GPIO_INPUT|GPIO_FLOAT|GPIO_SPEED_100MHz|GPIO_OPENDRAIN|GPIO_PORTA|GPIO_PIN9)

/* PWM */
#define DIRECT_PWM_OUTPUT_CHANNELS  8

/* High-resolution timer — TIM4 Ch1 */
#define HRT_TIMER          4
#define HRT_TIMER_CHANNEL  1

/* PPM input — TIM4 Ch3, PB8 */
#define HRT_PPM_CHANNEL    3
#define GPIO_PPM_IN        (GPIO_ALT|GPIO_AF2|GPIO_PULLUP|GPIO_PORTB|GPIO_PIN8)

/* RC serial — USART6 */
#define RC_SERIAL_PORT               "/dev/ttyS2"
#define BOARD_SUPPORTS_RC_SERIAL_PORT_OUTPUT

/* DMA pool */
#define BOARD_DMA_ALLOC_POOL_SIZE  5120

#define BOARD_HAS_ON_RESET  1

#define BOARD_ENABLE_CONSOLE_BUFFER
#define BOARD_CONSOLE_BUFFER_SIZE  (1024*3)


__BEGIN_DECLS

#ifndef __ASSEMBLY__

extern void stm32_spiinitialize(void);
extern void stm32_usbinitialize(void);
extern void board_peripheral_reset(int ms);

#include <px4_platform_common/board_common.h>

#endif /* __ASSEMBLY__ */

__END_DECLS
