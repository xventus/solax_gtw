//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   solax.h
/// @author Petr Vanek

#pragma once

#include "mbcontroller.h"
#include "esp_modbus_common.h"

enum class ParamCID
{ 
    PvVoltage1 = 0,
    PvVoltage2,
    PvCurrent1,
    PvCurrent2,
    Powerdc1,
    Powerdc2,
    BatVoltage_Charge1,
    BatCurrent_Charge1,
    Batpower_Charge1,
    TemperatureBat,
    BatCap,
    FeedinPower, // Power to grid
    GridPower_R,
    GridPower_S,
    GridPower_T,
    Etoday_togrid,
    Temperature, // radiator temperature
    RunMode,
    BDCStatus,
    GridStatus,
    MPPTCount
}; //

// Parameter definition for reading the Input register using FC4
const mb_parameter_descriptor_t solax_parameters[] = {

    {.cid = static_cast<uint16_t>(ParamCID::PvVoltage1),
     .param_key = "PvVoltage1",
     .param_units = "V",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x0003,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_U16,
     .param_size = PARAM_SIZE_U16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},
    {.cid = static_cast<uint16_t>(ParamCID::PvVoltage2),
     .param_key = "PvVoltage2",
     .param_units = "V",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x0004,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_U16,
     .param_size = PARAM_SIZE_U16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},

    {.cid = static_cast<uint16_t>(ParamCID::PvCurrent1),
     .param_key = "PvCurrent1",
     .param_units = "A",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x0005,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_U16,
     .param_size = PARAM_SIZE_U16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},

    {.cid = static_cast<uint16_t>(ParamCID::PvCurrent2),
     .param_key = "PvCurrent2",
     .param_units = "A",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x0006,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_I16_AB,
     .param_size = PARAM_SIZE_I16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},

    {.cid = static_cast<uint16_t>(ParamCID::Powerdc1),
     .param_key = "Powerdc1",
     .param_units = "W",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x000A,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_U16,
     .param_size = PARAM_SIZE_U16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},

    {.cid = static_cast<uint16_t>(ParamCID::Powerdc2),
     .param_key = "Powerdc2",
     .param_units = "W",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x000B,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_U16,
     .param_size = PARAM_SIZE_U16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},

    {.cid = static_cast<uint16_t>(ParamCID::BatVoltage_Charge1),
     .param_key = "BatVoltage_Charge1",
     .param_units = "V",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x0014,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_I16_AB,
     .param_size = PARAM_SIZE_I16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},

    {.cid = static_cast<uint16_t>(ParamCID::BatCurrent_Charge1),
     .param_key = "BatCurrent_Charge1",
     .param_units = "A",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x0015,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_I16_AB,
     .param_size = PARAM_SIZE_I16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},

    {.cid = static_cast<uint16_t>(ParamCID::Batpower_Charge1),
     .param_key = "Batpower_Charge1",
     .param_units = "W",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x0016,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_I16_AB,
     .param_size = PARAM_SIZE_I16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},

    {.cid = static_cast<uint16_t>(ParamCID::TemperatureBat),
     .param_key = "TemperatureBat",
     .param_units = "C",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x0018,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_I16_AB,
     .param_size = PARAM_SIZE_I16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},

    {.cid = static_cast<uint16_t>(ParamCID::BatCap),
     .param_key = "BattCap",
     .param_units = "%",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x001C,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_U16,
     .param_size = PARAM_SIZE_U16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},

    {.cid = static_cast<uint16_t>(ParamCID::FeedinPower),
     .param_key = "FeedinPower",
     .param_units = "W",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x0046,
     .mb_size = 2,
     .param_offset = 0,
     .param_type = PARAM_TYPE_I32_ABCD,
     .param_size = PARAM_SIZE_I32,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},
    {.cid = static_cast<uint16_t>(ParamCID::GridPower_R),
     .param_key = "GridPower_R",
     .param_units = "W",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x006C,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_I16_AB,
     .param_size = PARAM_SIZE_I16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},
    {.cid = static_cast<uint16_t>(ParamCID::GridPower_S),
     .param_key = "GridPower_S",
     .param_units = "W",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x0070,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_I16_AB,
     .param_size = PARAM_SIZE_I16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},
    {.cid = static_cast<uint16_t>(ParamCID::GridPower_T),
     .param_key = "GridPower_T", // 1W int16
     .param_units = "W",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x0074,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_I16_AB,
     .param_size = PARAM_SIZE_I16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},
    {.cid = static_cast<uint16_t>(ParamCID::Etoday_togrid),
     .param_key = "Etoday_togrid", // 0.1kWh uint16
     .param_units = "W",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x0050,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_U16,
     .param_size = PARAM_SIZE_U16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},
    {.cid = static_cast<uint16_t>(ParamCID::Temperature),
     .param_key = "Temperature",
     .param_units = "C",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x0008,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_I16_AB,
     .param_size = PARAM_SIZE_I16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},
    {.cid = static_cast<uint16_t>(ParamCID::RunMode),
     .param_key = "RunMode",
     .param_units = "-",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x0009,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_U16,
     .param_size = PARAM_SIZE_U16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},
    {.cid = static_cast<uint16_t>(ParamCID::BDCStatus),
     .param_key = "BDCStatus",
     .param_units = "-",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x0019,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_U16,
     .param_size = PARAM_SIZE_U16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},
    {.cid = static_cast<uint16_t>(ParamCID::GridStatus),
     .param_key = "GridStatus", // 0: OnGrid 1: OffGrid
     .param_units = "-",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x001A,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_U16,
     .param_size = PARAM_SIZE_U16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ},
    {.cid = static_cast<uint16_t>(ParamCID::MPPTCount),
     .param_key = "MPPTCount",
     .param_units = "-",
     .mb_slave_addr = 1,
     .mb_param_type = MB_PARAM_INPUT,
     .mb_reg_start = 0x001B,
     .mb_size = 1,
     .param_offset = 0,
     .param_type = PARAM_TYPE_U16,
     .param_size = PARAM_SIZE_U16,
     .param_opts = {0, 0, 0},
     .access = PAR_PERMS_READ}
};