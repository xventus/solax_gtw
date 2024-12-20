//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   mqtt_task.cpp
/// @author Petr Vanek

#include <cstdint>
#include <atomic>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "cJSON.h"
#include "mqtt_task.h"
#include "literals.h"
#include "application.h"
#include "json_serializer.h"
#include "key_val.h"



MqttTask::MqttTask() : _mqttClient(nullptr), _mqttInitialized(false)
{
    _queue = xQueueCreate(2, sizeof(QueueModbusPacket));
}

bool MqttTask::init(std::shared_ptr<ConnectionManager> connMgr, 
					const char * name, UBaseType_t priority, const configSTACK_DEPTH_TYPE stackDepth )
{
	bool rc = false;
	_connectionManager = connMgr;
	rc = RPTask::init(name, priority, stackDepth);
	return rc;
}

MqttTask::~MqttTask()
{
    done();
    if (_queue)
        vQueueDelete(_queue);
}

void MqttTask::initializeMqttClient()
{
    // Only create the client if it hasn't been created
    if (!_mqttClient) {
        _mqttClient = std::make_unique<Mqtt>();
        
        // Set up MQTT callbacks
        _mqttClient->registerConnectedCallback([&]() {
            ESP_LOGI(LOG_TAG, "Connected to MQTT broker");
            if (_connectionManager) _connectionManager->setMqttActive();
            Application::getInstance()->getWebTask()->log("Connected to MQTT broker");
            
        });
        
        _mqttClient->registerDisconnectedCallback([&]() {
            ESP_LOGI(LOG_TAG, "Disconnected from MQTT broker");
             if (_connectionManager) _connectionManager->setMqttDeactive(); 
             Application::getInstance()->getWebTask()->log("Disconnected from MQTT broker");
        });


        KeyVal &kv = KeyVal::getInstance();
        std::string mqtt;
        mqtt = "mqtt://";
        mqtt += kv.readString(literals::kv_mqtt);
        if (!_mqttClient->init(mqtt, kv.readString(literals::kv_user), kv.readString(literals::kv_passwd))) {
            ESP_LOGE(LOG_TAG, "Failed to initialize MQTT client");
            Application::getInstance()->getWebTask()->log("Failed to initialize MQTT client");
        }
    }
}


void MqttTask::doneMqttClient()
{
    // Only create the client if it hasn't been created
    if (_mqttClient) {
        _mqttClient.reset();
    }  
}


void MqttTask::loop()
{
    QueueModbusPacket packet; 
    KeyVal &kv = KeyVal::getInstance();
    auto topic = kv.readString(literals::kv_topic,"solax/data");

    while (true) { // Loop forever

        // Check connection status
        if (_connectionManager && _connectionManager->isConnected()) {
            if (!_mqttInitialized) {
                initializeMqttClient(); // Initialize MQTT only once upon connection
                _mqttInitialized = true;
            }
        } else {
            // Connection lost
            if (_mqttInitialized) {
                // Clean up the MQTT client if it was previously initialized
                doneMqttClient();
            }
            _mqttInitialized = false;
        }

        auto res = xQueueReceive(_queue, (void *)&packet, 0);
        if (res == pdTRUE) {
            // Process received packet
            if (_connectionManager && _connectionManager->isMqttActive()) {
                auto msg = JsonSerializer::serializeModbusPacket(packet);
                if (!_mqttClient->publish(topic, msg)) {
                    ESP_LOGE(LOG_TAG, "Failed to publish message");
                } else {
                    ESP_LOGI(LOG_TAG, "Message published: %s", msg.c_str());
                }
            }
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}


void  MqttTask::updateData(const QueueModbusPacket& msg)
    {
        if (_queue)
        {
            xQueueSendToBack(_queue, (void *)&msg, 0);
        }
    }