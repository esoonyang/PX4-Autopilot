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

#pragma once

/*
 * HAKRC F405 V2 DMA channel/stream selections.
 *
 * The board uses the same STM32F405RGT6 as the Omnibus F4 SD so the
 * DMA map is identical.
 *
 * DMA1 usage:
 *   Stream 1 Ch3  TIM2_UP   (DSHOT motor DMA)
 *   Stream 2 Ch5  TIM3_UP   (DSHOT motor DMA)
 *   Stream 3 Ch0  SPI2_RX   (SD card)
 *   Stream 4 Ch0  SPI2_TX   (SD card)
 *
 * DMA2 usage:
 *   Stream 0 Ch3  SPI1_RX   (IMU)
 *   Stream 1 Ch4  USART6_RX (RC serial)
 *   Stream 2 Ch4  USART1_RX (GPS)
 *   Stream 3 Ch3  SPI1_TX   (IMU)
 *   Stream 6 Ch4  SDIO
 */

// DMA1
#define DMACHAN_SPI2_RX    DMAMAP_SPI2_RX     // DMA1, Stream 3, Channel 0
#define DMACHAN_SPI2_TX    DMAMAP_SPI2_TX     // DMA1, Stream 4, Channel 0

// DMA2
#define DMACHAN_SPI1_RX    DMAMAP_SPI1_RX_1   // DMA2, Stream 0, Channel 3
#define DMAMAP_USART6_RX   DMAMAP_USART6_RX_1 // DMA2, Stream 1, Channel 4
#define DMAMAP_USART1_RX   DMAMAP_USART1_RX_1 // DMA2, Stream 2, Channel 4
#define DMACHAN_SPI1_TX    DMAMAP_SPI1_TX_1   // DMA2, Stream 3, Channel 3
#define DMAMAP_SDIO        DMAMAP_SDIO_2      // DMA2, Stream 6, Channel 4
