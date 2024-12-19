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
#include <string_view>
#include "log_queue_data.h"


class WebTask : public RPTask
{
public:
 enum class Mode {
	    Setting,     
        None, 	
        Unknown       };

	WebTask();
	virtual ~WebTask();
	void command(Mode mode);
	void log(std::string_view message);
	bool init(std::shared_ptr<ConnectionManager> connMgr, const char * name, UBaseType_t priority = tskIDLE_PRIORITY, const configSTACK_DEPTH_TYPE stackDepth = configMINIMAL_STACK_SIZE);

	
protected:
	void loop() override;

private:
	static constexpr const char *LOG_TAG = "WEB";
	Mode            _mode {Mode::Unknown};
	QueueHandle_t 	_queue;
	QueueHandle_t 	_queueLog;
	std::shared_ptr<ConnectionManager> _connectionManager;

};
