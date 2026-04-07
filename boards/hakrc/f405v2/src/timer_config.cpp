/****************************************************************************
 *
 *   Copyright (C) 2024 PX4 Development Team. All rights reserved.
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
 * Timer / PWM output configuration for the HAKRC F405 V2
 *
 * Motor outputs (DShot capable — TIM3 and TIM2 with DMA):
 *   Ch1  TIM3_CH3  PB0   M1 (front, CCW)
 *   Ch2  TIM3_CH4  PB1   M2 (right, CW)
 *   Ch3  TIM2_CH4  PA3   M3 (rear,  CCW)
 *   Ch4  TIM2_CH3  PA2   M4 (left,  CW)
 *
 * Servo outputs (standard PWM — TIM8, no DMA):
 *   Ch5  TIM8_CH3  PC8   S1
 *   Ch6  TIM8_CH4  PC9   S2
 *   Ch7  TIM8_CH1  PC6   S3  (pin shared with UART6 TX)
 *   Ch8  TIM8_CH2  PC7   S4  (pin shared with UART6 RX)
 *
 * DMA assignments:
 *   TIM3_UP : DMA1 Stream 2 Channel 5
 *   TIM2_UP : DMA1 Stream 1 Channel 3
 *   TIM8    : no DMA (servo PWM only, not DShot)
 */

#include <px4_arch/io_timer_hw_description.h>

constexpr io_timers_t io_timers[MAX_IO_TIMERS] = {
	initIOTimer(Timer::Timer3, DMA{DMA::Index1, DMA::Stream2, DMA::Channel5}),
	initIOTimer(Timer::Timer2, DMA{DMA::Index1, DMA::Stream1, DMA::Channel3}),
	initIOTimer(Timer::Timer8),
};

constexpr timer_io_channels_t timer_io_channels[MAX_TIMER_IO_CHANNELS] = {
	/* Motors — TIM3 */
	initIOTimerChannel(io_timers, {Timer::Timer3, Timer::Channel3}, {GPIO::PortB, GPIO::Pin0}),
	initIOTimerChannel(io_timers, {Timer::Timer3, Timer::Channel4}, {GPIO::PortB, GPIO::Pin1}),
	/* Motors — TIM2 */
	initIOTimerChannel(io_timers, {Timer::Timer2, Timer::Channel4}, {GPIO::PortA, GPIO::Pin3}),
	initIOTimerChannel(io_timers, {Timer::Timer2, Timer::Channel3}, {GPIO::PortA, GPIO::Pin2}),
	/* Servos — TIM8 */
	initIOTimerChannel(io_timers, {Timer::Timer8, Timer::Channel3}, {GPIO::PortC, GPIO::Pin8}),
	initIOTimerChannel(io_timers, {Timer::Timer8, Timer::Channel4}, {GPIO::PortC, GPIO::Pin9}),
	initIOTimerChannel(io_timers, {Timer::Timer8, Timer::Channel1}, {GPIO::PortC, GPIO::Pin6}),
	initIOTimerChannel(io_timers, {Timer::Timer8, Timer::Channel2}, {GPIO::PortC, GPIO::Pin7}),
};

constexpr io_timers_channel_mapping_t io_timers_channel_mapping =
	initIOTimerChannelMapping(io_timers, timer_io_channels);
