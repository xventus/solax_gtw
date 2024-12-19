//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   modbus_task.h
/// @author Petr Vanek

#pragma once

#include <memory>
#include "hardware.h"
#include "rptask.h"
#include "mqtt.h"
#include "literals.h"
#include "connection_manager.h"
#include "modbus_master.h"
#include "mqtt_queue_data.h"


class ModbusTask : public RPTask
{
public:
 	ModbusTask();
	virtual ~ModbusTask();
	bool init(std::shared_ptr<ConnectionManager> connMgr, const char * name, UBaseType_t priority = tskIDLE_PRIORITY, const configSTACK_DEPTH_TYPE stackDepth = configMINIMAL_STACK_SIZE);

protected:
	void loop() override;
	
private:
	static constexpr const char* LOG_TAG = "ModbusTsk";
	void* _mbHandler;  
	QueueHandle_t 	_queue;
	std::shared_ptr<ConnectionManager> _connectionManager;
  
};
