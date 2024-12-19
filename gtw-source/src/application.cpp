//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   application.h
/// @author Petr Vanek

#include <esp_netif.h>
#include <esp_event.h>

#include "application.h"
#include "hardware.h"
#include "content_file.h"
#include "driver/uart.h"
#include "key_val.h"
#include "button.h"

// global application instance as singleton and instance acquisition.

Application *Application::getInstance()
{
    static Application instance;
    return &instance;
}

Application::Application()
{
}

Application::~Application()
{
    done();
}

void Application::init()
{
    // Begin initialization of core functions so that individual
    // wrappers no longer need to execute

    // initi file system
    ConentFile::initFS();

    // initialize NVS
    KeyVal &kv = KeyVal::getInstance();
    kv.init(literals::kv_namespace, true, false);

    // initialize newtwork interfaces
    ESP_ERROR_CHECK(esp_netif_init());

    // default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
}

void Application::run()
{
    // task create
    do
    {

        _connectionManager = std::make_shared<ConnectionManager>();

        if (!_connectionManager)
        {
            ESP_LOGE(LOG_TAG, "Failed to create ConnectionManager.");
        }

         if (!_heartBeat.init(literals::tsk_led, tskIDLE_PRIORITY + 1ul, configMINIMAL_STACK_SIZE*10))
            break;

        if (!_ethernetTask.init(_connectionManager, literals::tsk_eth, tskIDLE_PRIORITY + 1ul, configMINIMAL_STACK_SIZE * 10))
            break;

        if (!_mqttTask.init(_connectionManager, literals::mqt_eth, tskIDLE_PRIORITY + 1ul, configMINIMAL_STACK_SIZE * 10))
            break;

        if (!_mbTask.init(_connectionManager, literals::tsk_modbus, tskIDLE_PRIORITY + 1ul, configMINIMAL_STACK_SIZE * 10))
            break;

        if (!_wifiTask.init(_connectionManager, literals::tsk_web, tskIDLE_PRIORITY + 1ul, 4086))
            break;

        if (!_webTask.init(_connectionManager, literals::tsk_wifi, tskIDLE_PRIORITY + 1ul, 4086))
            break;

         if (!_resetTask.init(literals::tsk_rst, tskIDLE_PRIORITY + 1ul, configMINIMAL_STACK_SIZE))
            break;

      
    } while (false);
}

void Application::done()
{
    // fail
    KeyVal &kv = KeyVal::getInstance();
    kv.done();
}

Application *Application::operator->()
{
    return Application::getInstance();
}

Application *Application::operator->() const
{
    return Application::getInstance();
}
