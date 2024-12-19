//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   main.h
/// @author Petr Vanek


#include "application.h"
#include "hardware.h"
#include <esp_log.h>
#include <esp_event.h>
#include <esp_eth.h>
#include <esp_netif.h>
#include <esp_system.h>

extern "C"
{
    void app_main(void);
}

void app_main()
{

     Application::getInstance()->init();
     Application::getInstance()->run();

    return;
}




