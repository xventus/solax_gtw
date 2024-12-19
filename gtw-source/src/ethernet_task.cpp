
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   ethernet_task.cpp
/// @author Petr Vanek

#include <cstdint>
#include <atomic>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "ethernet_client.h"
#include "ethernet_task.h"
#include "literals.h"
#include "application.h"

EthernetTask::EthernetTask()
{
	_queue = xQueueCreate(2, sizeof(int));
}

EthernetTask::~EthernetTask()
{
	done();
	if (_queue)
		vQueueDelete(_queue);
}

bool EthernetTask::init(std::shared_ptr<ConnectionManager> connMgr, const char *name, UBaseType_t priority, const configSTACK_DEPTH_TYPE stackDepth)
{
	bool rc = false;
	_connectionManager = connMgr;
	rc = RPTask::init(name, priority, stackDepth);
	return rc;
}


void EthernetTask::loop()
{

	std::uint16_t info{0};
	auto ip_info = std::make_shared<esp_netif_ip_info_t>();
	std::mutex ip_mutex;

	int mode{0};
	EthernetClient ec;
	Application::getInstance()->getWebTask()->log("Waiting for ethernet");
	ec.registerConnectedCallback([this, &info, ip_info, &ip_mutex](const ip_event_got_ip_t &event)

								 {
									 ESP_LOGI(LOG_TAG, "ETHIP:" IPSTR, IP2STR(&event.ip_info.ip));
									 ESP_LOGI(LOG_TAG, "ETHMASK:" IPSTR, IP2STR(&event.ip_info.netmask));
									 ESP_LOGI(LOG_TAG, "ETHGW:" IPSTR, IP2STR(&event.ip_info.gw));

									 Application::getInstance()->getWebTask()->log("Ethernet connected");
									
										if (_connectionManager) 
										{	
											_connectionManager->setConnected();
										}

										 std::lock_guard<std::mutex> lock(ip_mutex);
										 *ip_info = event.ip_info;
										 info = 1;
										 
									
								 });

	ec.registerDisconnectedCallback([this, &info, &ip_mutex]()
									{ 
										 {
											if (_connectionManager) 
											{	
												_connectionManager->setDisconnected();
												Application::getInstance()->getWebTask()->log("No ethernet");
											}

                                         	std::lock_guard<std::mutex> lock(ip_mutex); 
											info = 1;
										 }

										ESP_LOGI(LOG_TAG, "Disconnected"); });
	ec.init(ETH_RESET, ETH_MDC, ETH_MDIO, ETH_CLK, false);

	while (true)
	{ // Loop forever
		auto res = xQueueReceive(_queue, (void *)&mode, 0);
		if (res == pdTRUE)
		{
		}
		if (_connectionManager && !_connectionManager->isConnected()) Application::getInstance()->getWebTask()->log("Waiting for ethernet");
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}
