//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   log_queue_data.h
/// @author Petr Vanek

#pragma once


#include <stdint.h>
#include <string.h>


#define MAX_LOG_LINE 100

struct LogData   
{
    char msg[MAX_LOG_LINE];   
};