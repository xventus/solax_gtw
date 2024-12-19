
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   reset_task.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "reset_task.h"
#include "esp_log.h"
#include "key_val.h"
#include "literals.h"

ResetTask::ResetTask() { 
	_queue = xQueueCreate( 1 , sizeof(int));
}

ResetTask::~ResetTask() {
	done();
	if (_queue) vQueueDelete(_queue);
}


void ResetTask::loop() {

    while (true) {
        int req;
        if (xQueueReceive(_queue, &req, 0) == pdTRUE) {
          if (req==1) {
                ESP_LOGI(LOG_TAG,"RESET !!");
                vTaskDelay(pdMS_TO_TICKS(2000));  
    			esp_restart();  
          }
        }
       vTaskDelay(pdMS_TO_TICKS(1000));  
    }
}

 void ResetTask::reset() {

    int i = 1;
	if (_queue) {
        xQueueSendToBack(_queue, &i, 0); 
    }
 }