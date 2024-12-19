//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   json_serializer.h
/// @author Petr Vanek

#pragma once

#include <string>
#include "cJSON.h"
#include "mqtt_queue_data.h"

class JsonSerializer {
public:
    

    static std::string serializeModbusPacket(const QueueModbusPacket& packet) {
        cJSON* root = cJSON_CreateObject();
        if (root == nullptr) {
            return "{}"; // Return an empty JSON object on failure
        }

        cJSON_AddNumberToObject(root, "value", packet.value);
        cJSON_AddStringToObject(root, "unit", packet.unit);
        cJSON_AddStringToObject(root, "name", packet.name);

        char* jsonStr = cJSON_PrintUnformatted(root);
        cJSON_Delete(root);

        if (jsonStr == nullptr) {
            return "{}"; // Return an empty JSON object on failure
        }

        std::string result(jsonStr);
        cJSON_free(jsonStr); // Free the allocated memory

        return result;
    }
};