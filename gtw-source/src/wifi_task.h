//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   web_task.h
/// @author Petr Vanek

#pragma once

#include "hardware.h"
#include "rptask.h"
#include "access_point.h"
#include "literals.h"
#include "connection_manager.h"

class WifiTask : public RPTask
{
public:
 enum class Mode {
	    AP,     // Access Point 
        Client, // STA
        Stop    // Stopped
    };

	WifiTask();
	virtual ~WifiTask();
	void switchMode(Mode mode);
	bool init(std::shared_ptr<ConnectionManager> connMgr, const char * name, UBaseType_t priority = tskIDLE_PRIORITY, const configSTACK_DEPTH_TYPE stackDepth = configMINIMAL_STACK_SIZE);

protected:
	void loop() override;

private:
    static constexpr const char *LOG_TAG = "WIFI";
	Mode            _mode {Mode::Stop};
	QueueHandle_t 	_queue;
	std::shared_ptr<ConnectionManager> _connectionManager;
};
