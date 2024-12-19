//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   literals.h
/// @author Petr Vanek

#pragma once

#include <stdio.h>
#include <string_view>


class literals
{
public:

    // internal - do not modify
    
    // internal task name
    static constexpr const char *tsk_modbus{"MBTSK"};
    static constexpr const char *tsk_eth{"ETHTSK"};
    static constexpr const char *mqt_eth{"MQTTSK"};
    static constexpr const char *tsk_led{"LEDTSK"};
    static constexpr const char *tsk_web{"WEBTSK"};
    static constexpr const char *tsk_wifi{"WIFITSK"};
    static constexpr const char *tsk_rst{"RSTTSK"};

    // AP definition
    static constexpr const char *ap_name{"SOLAX-GATEWAY"};
    static constexpr const char *ap_passwd{""};

    // KV & files
    static constexpr const char *kv_namespace{"slxgtw"};

    static constexpr const char *kv_ip{"ip"};           ///  192.168.x.x
    static constexpr const char *kv_port{"port"};       ///  modbus port 502
    static constexpr const char *kv_mqtt{"mqtt"};       ///  "mqtt://x.x.x.x"
    static constexpr const char *kv_user{"user"};       ///  mqtt user 
    static constexpr const char *kv_passwd{"passwd"};   ///  mqtt password
    static constexpr const char *kv_refresh{"refresh"}; ///  modbus -> mqtt refresh
  

    // spiffs filenames
    static constexpr const char *kv_fl_index{"/spiffs/index.html"}; 
    static constexpr const char *kv_fl_apb{"/spiffs/ap_beg.html"};
    static constexpr const char *kv_fl_ape{"/spiffs/ap_end.html"}; 
    static constexpr const char *kv_fl_style{"/spiffs/style.css"}; 
    static constexpr const char *kv_fl_finish{"/spiffs/finish.html"};
    


};