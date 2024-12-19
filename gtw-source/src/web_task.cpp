
//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   web_task.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

#include "application.h"
#include "web_task.h"
#include "http_server.h"
#include "key_val.h"
#include "content_file.h"
#include "http_request.h"
#include <cJSON.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

WebTask::WebTask()
{
	_queue = xQueueCreate(2, sizeof(int));
	_queueLog = xQueueCreate(2, sizeof(LogData));
}

WebTask::~WebTask()
{
	done();
	if (_queue)
		vQueueDelete(_queue);

	if (_queueLog)
		vQueueDelete(_queueLog);
}

void WebTask::loop()
{

	HttpServer server;
	LogData log;
	memset(&log, 0, sizeof(log));
	while (true)
	{ // Loop forever

		auto res = xQueueReceive(_queueLog, (void *)&log, 0);
		if (res == pdTRUE)
		{
			ESP_LOGI(LOG_TAG, "Log recv >%s<", log.msg);
		}

		int receivedMode;
		res = xQueueReceive(_queue, (void *)&receivedMode, 0);
		if (res == pdTRUE)
		{
			Mode mode = static_cast<Mode>(receivedMode);
			if (mode == Mode::None)
			{
				ESP_LOGI(LOG_TAG, "mode == Mode::None");
				server.stop();
			}
			else if (mode == Mode::Setting)
			{
				server.stop();
				server.start();

				// AP main page
				server.registerUriHandler("/", HTTP_GET, [](httpd_req_t *req) -> esp_err_t
										  {

				auto replaceAll = [](std::string& str, const std::string& from, const std::string& to) {
						if(from.empty())
							return;
						size_t start_pos = 0;
						while((start_pos = str.find(from, start_pos)) != std::string::npos) {
							str.replace(start_pos, from.length(), to);
							start_pos += to.length(); 
						}
					};
					
					KeyVal& kv = KeyVal::getInstance();
					ConentFile ap1cntb(literals::kv_fl_apb);
					ConentFile ap1cnte(literals::kv_fl_ape);
    				auto contnetb = ap1cntb.readContnet();
					
					if (contnetb.empty()) {
						httpd_resp_send_custom_err(req, literals::kv_fl_apb, "initialize SPIF first!");
					}

					replaceAll(contnetb, "%IP_ADDRESS%", kv.readString(literals::kv_ip,"192.168.2.254"));
					replaceAll(contnetb, "%MODBUS_PORT%", kv.readString(literals::kv_port,"502"));
					replaceAll(contnetb, "%MQTT_BROKER%", kv.readString(literals::kv_mqtt,"192.168.2.20"));
					replaceAll(contnetb, "%BROKER_USER%", kv.readString(literals::kv_user,""));
					replaceAll(contnetb, "%BROKER_PASSWD%", kv.readString(literals::kv_passwd,""));
					replaceAll(contnetb, "%REFRESH_RATE%", kv.readString(literals::kv_refresh,"100"));

					contnetb += ap1cnte.readContnet();
 					httpd_resp_send(req, contnetb.c_str(), contnetb.length());
                    return ESP_OK; });

				server.registerUriHandler("/style.css", HTTP_GET, [](httpd_req_t *req) -> esp_err_t
										  {
					 httpd_resp_set_type(req, "text/css");
					 ConentFile ap1cnt(literals::kv_fl_style);
    				 auto contnet = ap1cnt.readContnet();
                    httpd_resp_send(req, contnet.c_str(), contnet.length());
                   
                    return ESP_OK; });

				// AP setting answer
				server.registerUriHandler("/", HTTP_POST, [](httpd_req_t *req) -> esp_err_t
										  {
					char content[300] = {0}; 
					int received = httpd_req_recv(req, content, sizeof(content) - 1);
					if (received <= 0) { 
						if (received == HTTPD_SOCK_ERR_TIMEOUT) {
							httpd_resp_send_408(req);
						}
						return ESP_FAIL;
					}

					// parse response & store configuration
					std::map<std::string, std::string> formData = HttpReqest::parseFormData(std::string(content));
					KeyVal& kv = KeyVal::getInstance();
					
					kv.writeString(literals::kv_ip, HttpReqest::getValue(formData, literals::kv_ip).c_str());
					kv.writeString(literals::kv_port, HttpReqest::getValue(formData, literals::kv_port).c_str());
					kv.writeString(literals::kv_mqtt, HttpReqest::getValue(formData, literals::kv_mqtt).c_str());
					kv.writeString(literals::kv_user, HttpReqest::getValue(formData, literals::kv_user).c_str());
					kv.writeString(literals::kv_passwd, HttpReqest::getValue(formData, literals::kv_passwd).c_str());
					kv.writeString(literals::kv_refresh, HttpReqest::getValue(formData, literals::kv_refresh).c_str());
					
					// Response & swith mode 
					ConentFile respContnetDialog(literals::kv_fl_finish);
    				auto finish = respContnetDialog.readContnet();
					httpd_resp_send(req, finish.c_str(), finish.length());

					 Application::getInstance()->getResetTask()->reset();

					return ESP_OK; });

				server.registerUriHandler("/log", HTTP_GET, [&log](httpd_req_t *req) -> esp_err_t
										  {	
							httpd_resp_set_type(req, "text/plain");
							httpd_resp_send(req, log.msg , strlen(log.msg));
					 		
                    		return ESP_OK; });
			}
		}

		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

void WebTask::command(Mode mode)
{
	if (_queue)
	{
		int modeToSend = static_cast<int>(mode);
		xQueueSendToBack(_queue, (void *)&modeToSend, 0);
	}
}

void WebTask::log(std::string_view message)
{
	LogData log;

	if (_connectionManager && _connectionManager->isAPActive() && _queueLog && !message.empty())
	{
		memset(&log, 0, sizeof(log));
		if (message.length() > sizeof(log.msg))
		{
			strncpy(log.msg, message.data(), sizeof(log.msg) - 3);
			strcat(log.msg, "...");
		}
		else
		{
			strncpy(log.msg, message.data(), sizeof(log.msg));
		}
		xQueueSendToBack(_queueLog, (void *)&log, 0);
	}
}

bool WebTask::init(std::shared_ptr<ConnectionManager> connMgr, 
					const char * name, UBaseType_t priority, const configSTACK_DEPTH_TYPE stackDepth )
{
	bool rc = false;
	_connectionManager = connMgr;
	rc = RPTask::init(name, priority, stackDepth);
	return rc;
}