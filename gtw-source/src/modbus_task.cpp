//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   modbus_task.h
/// @author Petr Vanek

#include <cstdint>
#include <atomic>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "modbus_task.h"
#include "literals.h"
#include "application.h"
#include "esp_modbus_master.h"
#include "sdkconfig.h"
#include "solax.h"
#include "application.h"
#include "esp_modbus_common.h"
#include "key_val.h"
#include "literals.h"
#include "esp_log.h"

ModbusTask::ModbusTask() : _mbHandler(nullptr) 
{
    _queue = xQueueCreate(2, sizeof(int));
}

bool ModbusTask::init(std::shared_ptr<ConnectionManager> connMgr, 
					const char * name, UBaseType_t priority, const configSTACK_DEPTH_TYPE stackDepth )
{
	bool rc = false;
	_connectionManager = connMgr;
	rc = RPTask::init(name, priority, stackDepth);
	return rc;
}

ModbusTask::~ModbusTask()
{
    done();
    if (_queue)
        vQueueDelete(_queue);
}


void ModbusTask::loop()
{

    ModbusMaster mb(solax_parameters);
    int mode; // Variable to receive data from the queue
    
    KeyVal &kv = KeyVal::getInstance();
   
    auto tick = std::stoi(kv.readString(literals::kv_refresh,"100"));
    if (tick < 0 || tick > std::numeric_limits<uint32_t>::max()) {
        tick = 100;
    }
    
    ESP_LOGI(LOG_TAG, "MODBUS TICK %d", tick);

    while (true) { // Loop forever
        
        auto res = xQueueReceive(_queue, (void *)&mode, 0);
        if (res == pdTRUE) {
           
        }

        // Check connection status
        // ETH ready & MQTT connected
        if (_connectionManager && _connectionManager->isConnected() && _connectionManager->isMqttActive()) {
            
          if (!mb.isActive())  {
             mb.clear();

             KeyVal &kv = KeyVal::getInstance();

            auto value = std::stoi(kv.readString(literals::kv_port));
            if (value < 0 || value > std::numeric_limits<uint16_t>::max()) {
                value = 502;
            }

             mb.withPort( static_cast<uint16_t>(value)).addSlave(kv.readString(literals::kv_ip));
             if (!mb.start()) 
             {
                Application::getInstance()->getWebTask()->log("Modbus master start failed");
                Application::getInstance()->getLEDTask()->mode(BlinkMode::MODBUS_ERR);
             } else {
                Application::getInstance()->getWebTask()->log("Modbus master started");
             }
          }

            if (mb.isActive()) {
                
                auto ok = mb.fncReader(tick, [&mb,this] (QueueModbusPacket &pck) -> bool {
                    if (_connectionManager->isMqttActive()) {
                    Application::getInstance()->getMqttTask()->updateData(pck);
                    Application::getInstance()->getLEDTask()->mode(BlinkMode::WORK);
                    std::string log;
                    log = pck.name;
                    log += " = " + std::to_string(pck.value);
                    Application::getInstance()->getWebTask()->log(log);
                    return true;
                    } 
                    Application::getInstance()->getLEDTask()->mode(BlinkMode::MODBUS_ERR);
                    return false;
                } );

                if (!ok) Application::getInstance()->getLEDTask()->mode(BlinkMode::MODBUS_ERR);
            }           
        }

        if (mb.isActive() && _connectionManager && !_connectionManager->isConnected()) {
            Application::getInstance()->getLEDTask()->mode(BlinkMode::ERROR);
            mb.stop();
        }

        vTaskDelay((5*tick) / portTICK_PERIOD_MS);
    }
}

