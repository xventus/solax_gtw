//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   mqtt_queue_data.h
/// @author Petr Vanek

#pragma once


#include <stdint.h>
#include <string.h>

#define MAX_UNIT_LENGTH 5
#define MAX_PARAM_NAME_LENGTH 50

struct QueueModbusPacket {
    int32_t value;                          // value
    char unit[MAX_UNIT_LENGTH];             // unit
    char name[MAX_PARAM_NAME_LENGTH];       // name
};