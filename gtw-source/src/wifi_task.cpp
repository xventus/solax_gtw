
//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   wifi_task.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

#include "esp_netif.h"
#include "lwip/netif.h"

#include "wifi_task.h"
#include "key_val.h"
#include "literals.h"
#include "application.h"
#include "button.h"

WifiTask::WifiTask()
{
	_queue = xQueueCreate(2, sizeof(int));
}

WifiTask::~WifiTask()
{
	done();
	if (_queue)
		vQueueDelete(_queue);
}


bool WifiTask::init(std::shared_ptr<ConnectionManager> connMgr, 
					const char * name, UBaseType_t priority, const configSTACK_DEPTH_TYPE stackDepth )
{
	bool rc = false;
	_connectionManager = connMgr;
	rc = RPTask::init(name, priority, stackDepth);
	return rc;
}


void WifiTask::loop()
{
	WiFiAccessPoint wftt;
	bool processit = false;
	int receivedMode = static_cast<int>(Mode::Stop);
	Button jumper(CONFIG_JUMPER);

	while (true)
	{ // Loop forever

		auto res = xQueueReceive(_queue, (void *)&receivedMode, 0);
		if (res == pdTRUE)
		{
			processit = true;
		} 
		else 
		{
			if (jumper.isPressed() && receivedMode == static_cast<int>(Mode::Stop)) 
			{
				processit = true;
				receivedMode = static_cast<int>(Mode::AP);
			}

			if (!jumper.isPressed() && receivedMode == static_cast<int>(Mode::AP)) 
			{
				processit = true;
				receivedMode = static_cast<int>(Mode::Stop);
			}
		}

		if (processit)
		{
			processit = false;
			Mode mode = static_cast<Mode>(receivedMode);

			// Stop mode & check initial configuration
			if (mode == Mode::Stop)
			{
				ESP_LOGI(LOG_TAG, "STOP");
				// stop all previous wifi modes
				if (_connectionManager) _connectionManager->setAPDeactive();
				wftt.stop();
			}
			else if (mode == Mode::AP)
			{
				ESP_LOGI(LOG_TAG, "AP START");
				// AP start
				wftt.start(literals::ap_name, literals::ap_passwd);	
				Application::getInstance()->getWebTask()->command(WebTask::Mode::Setting);
				if (_connectionManager) _connectionManager->setAPActive();
			}
			else
			{
				ESP_LOGE(LOG_TAG, "invalid logic");
				// stop all previous wifi modes
				if (_connectionManager) _connectionManager->setAPDeactive();
				wftt.stop();
			}
		}

		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

void WifiTask::switchMode(Mode mode)
{
	if (_queue)
	{
		int modeToSend = static_cast<int>(mode);
		xQueueSendToBack(_queue, (void *)&modeToSend, 0);
	}
}

