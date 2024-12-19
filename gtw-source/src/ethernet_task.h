//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   ethernet_task.h
/// @author Petr Vanek

#pragma once

#include "hardware.h"
#include "rptask.h"
#include "ethernet_client.h"
#include "literals.h"
#include "connection_manager.h"
#include <memory>

class EthernetTask : public RPTask
{
public:
 	EthernetTask();
	virtual ~EthernetTask();
	bool init(std::shared_ptr<ConnectionManager> connMgr, const char * name, UBaseType_t priority = tskIDLE_PRIORITY, const configSTACK_DEPTH_TYPE stackDepth = configMINIMAL_STACK_SIZE);

protected:
	void loop() override;

private:
	static constexpr const char *LOG_TAG = "EthTask";
	QueueHandle_t 	_queue;
	std::shared_ptr<ConnectionManager> _connectionManager;
	
};
