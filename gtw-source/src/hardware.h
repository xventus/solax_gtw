//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   hardware.h
/// @author Petr Vanek

#pragma once

#include "driver/gpio.h"
#include "driver/uart.h"

/**
 * @brief Hardware definition, pin assignment, communication interfaces settings
 * 
 */

// Ethenrnet based on Microchip LAN8720
#define ETH_RESET       GPIO_NUM_5
#define ETH_MDC         GPIO_NUM_23
#define ETH_MDIO        GPIO_NUM_18
#define ETH_CLK         GPIO_NUM_17

// LEDs 
#define  HEART_BEAT_LED  GPIO_NUM_2     // LED between 3V3 and GPIO

// Jumper - WIFI AP
#define  CONFIG_JUMPER   GPIO_NUM_14    // JUMER between GND and GPIO

/*
I only

IO34
IO35
IO36 (VP)
IO39 (VN)

I/O

IO2
IO4
IO12
IO14
IO15
IO16
IO32
IO33
---

Pin Header

IO36    IO39
IO34    IO35 
IO16    IO32
IO33    IO12
IOI04   IO15
IO02    IO14
3V3     GND
3V3     GND

*/

