//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   modbus_master.h
/// @author Petr Vanek

#pragma once

#include <string_view>
#include <vector>
#include <string>
#include <span>
#include <functional>
#include <type_traits>
#include <limits>
#include <esp_netif.h>
#include "mbcontroller.h"
#include "esp_modbus_common.h"
#include "mqtt_queue_data.h"

class ModbusMaster
{
public:
    ModbusMaster(std::span<const mb_parameter_descriptor_t> params)
        : _params(params), _port{502} {}

    ~ModbusMaster()
    {
        stop();
        clear();
    }

    // Clear all slaves and reset the IP address table
    void clear()
    {
        _slaveIpAddressTable.clear();
        _slaveIps.clear();
    }

    // Set the Modbus TCP port
    ModbusMaster& withPort(uint16_t port)
    {
        _port = port;
        return *this;
    }

    // Add a slave IP address to the table
    ModbusMaster& addSlave(std::string_view ip)
    {
        if (!ip.empty())
        {
            _slaveIps.emplace_back(ip);
        }
        return *this;
    }

    // Start the Modbus communication
    bool start()
    {
        if (_slaveIps.empty())
        {
            ESP_LOGE(LOG_TAG, "No slave IPs provided. Cannot start Modbus Master.");
            return false;
        }

        for (const auto& ip : _slaveIps) {
            _slaveIpAddressTable.push_back(ip.c_str());
        }
        _slaveIpAddressTable.push_back(nullptr); // Ensure the end of the table is marked

        esp_err_t err = mbc_master_init_tcp(&_mbHandler);
        if (err != ESP_OK)
        {
            ESP_LOGE(LOG_TAG, "Failed to initialize Modbus Master: %s", esp_err_to_name(err));
            return false;
        }
        ESP_LOGI(LOG_TAG, "Modbus TCP Master initialized.");

        mb_communication_info_t comm_info = {};
        comm_info.ip_port = _port;
        comm_info.ip_addr_type = MB_IPV4;
        comm_info.ip_mode = MB_MODE_TCP;
        comm_info.ip_addr = static_cast<void*>(_slaveIpAddressTable.data());
        comm_info.ip_netif_ptr = (void*)esp_netif_get_default_netif();

        err = mbc_master_setup(&comm_info);
        if (err != ESP_OK)
        {
            ESP_LOGE(LOG_TAG, "Failed to set up Modbus Master: %s", esp_err_to_name(err));
            return false;
        }
        ESP_LOGI(LOG_TAG, "Modbus Master setup completed.");

        err = mbc_master_set_descriptor(_params.data(), _params.size());
        if (err != ESP_OK)
        {
            ESP_LOGE(LOG_TAG, "Failed to set Modbus parameter descriptors: %s", esp_err_to_name(err));
            return false;
        }

        err = mbc_master_start();
        if (err != ESP_OK)
        {
            ESP_LOGE(LOG_TAG, "Failed to start Modbus Master communication: %s", esp_err_to_name(err));
            return false;
        }
        ESP_LOGI(LOG_TAG, "Modbus Master communication started.");
        return true;
    }

    // Stop the Modbus Master
    esp_err_t stop()
    {
        if (_mbHandler)
        {
            esp_err_t err = mbc_master_destroy();
            if (err == ESP_OK)
            {
                ESP_LOGI(LOG_TAG, "Modbus Master stopped successfully.");
            }
            else
            {
                ESP_LOGE(LOG_TAG, "Failed to stop Modbus Master: %s", esp_err_to_name(err));
            }
            _mbHandler = nullptr;
            return err;
        }
        return ESP_OK;
    }

    // Update a Modbus message with provided data
    template <typename T>
    bool updateMessage(const char* unit, const char* name, T value, const std::function<bool(QueueModbusPacket&)>& processor)
    {
        static_assert(std::is_integral<T>::value, "Value must be an integral type");

        QueueModbusPacket modbus;

        strncpy(modbus.unit, unit, MAX_UNIT_LENGTH - 1);
        modbus.unit[MAX_UNIT_LENGTH - 1] = '\0';

        strncpy(modbus.name, name, MAX_PARAM_NAME_LENGTH - 1);
        modbus.name[MAX_PARAM_NAME_LENGTH - 1] = '\0';

        if (sizeof(T) > sizeof(int32_t))
        {
            if (value > std::numeric_limits<int32_t>::max() || value < std::numeric_limits<int32_t>::min())
            {
                return false; // Value out of range
            }
        }
        modbus.value = static_cast<int32_t>(value);

        return processor(modbus);
    }

bool fncReader(int32_t tick, const std::function<bool(QueueModbusPacket&)>& processor)
{
    auto rc = true;
    esp_err_t err = ESP_OK;
    const mb_parameter_descriptor_t* param_descriptor = nullptr;

    for (const auto& param : _params)
    {
        err = mbc_master_get_cid_info(param.cid, &param_descriptor);
        if ((err != ESP_ERR_NOT_FOUND) && (param_descriptor != nullptr))
        {
            switch (param.param_type)
            {
            case PARAM_TYPE_I32_ABCD:
            {
                int32_t value_i32;
                uint8_t t;
                // Cast param_key to char* explicitly
                err = mbc_master_get_parameter(param.cid, const_cast<char*>(param_descriptor->param_key), (uint8_t*)&value_i32, &t);
                if (err == ESP_OK)
                {
                    ESP_LOGI(LOG_TAG, "I32 %s: %ld", param_descriptor->param_key, value_i32);
                    if (!updateMessage(param_descriptor->param_units, param_descriptor->param_key, value_i32, processor))
                        return false;
                }
                break;
            }
            case PARAM_TYPE_I16_AB:
            {
                int16_t value_i16;
                uint8_t t;
                // Cast param_key to char* explicitly
                err = mbc_master_get_parameter(param.cid, const_cast<char*>(param_descriptor->param_key), (uint8_t*)&value_i16, &t);
                if (err == ESP_OK)
                {
                    ESP_LOGI(LOG_TAG, "I16 %s: %d", param_descriptor->param_key, value_i16);
                    if (!updateMessage(param_descriptor->param_units, param_descriptor->param_key, value_i16, processor))
                        return false;
                }
                break;
            }
            case PARAM_TYPE_U16:
            {
                uint16_t value_u16;
                uint8_t t;
                // Cast param_key to char* explicitly
                err = mbc_master_get_parameter(param.cid, const_cast<char*>(param_descriptor->param_key), (uint8_t*)&value_u16, &t);
                if (err == ESP_OK)
                {
                    ESP_LOGI(LOG_TAG, "U16 %s: %u", param_descriptor->param_key, value_u16);
                    if (!updateMessage(param_descriptor->param_units, param_descriptor->param_key, value_u16, processor))
                        return false;
                }
                break;
            }
            default:
                ESP_LOGW(LOG_TAG, "Unhandled parameter type for CID %d", param.cid);
                rc = false;
                break;
            }

            if (err != ESP_OK) 
            {
                ESP_LOGE(LOG_TAG, "some error occured EC: %d: ", err);
                rc = false;
                break;
            }

            vTaskDelay((5*tick) / portTICK_PERIOD_MS);
        } 
        else 
        {
            ESP_LOGE(LOG_TAG, "info error EC: %d: ", err);
            rc = false;
            break;
        }
    }

    return rc;
}

bool isActive() const 
{
    return _mbHandler;
}

private:
    static constexpr const char* LOG_TAG = "ModbusMaster";
    void* _mbHandler{nullptr};
    std::span<const mb_parameter_descriptor_t> _params;
    std::vector<const char*> _slaveIpAddressTable;
    std::vector<std::string> _slaveIps;
    uint16_t _port{502};
};
