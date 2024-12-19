//
// vim: ts=4 et
// Copyright (c) 2024 Petr Vanek, petr@fotoventus.cz
//
/// @file   ethernet_client.h
/// @author Petr Vanek

/*
Note:

    EthernetClient ec;
    ec.registerConnectedCallback([](const ip_event_got_ip_t &event){
            ESP_LOGI(">>>>>> EthernetClient", "ETHIP:" IPSTR, IP2STR(&event.ip_info.ip));
            ESP_LOGI(">>>>>> EthernetClient", "ETHMASK:" IPSTR, IP2STR(&event.ip_info.netmask));
            ESP_LOGI(">>>>>> EthernetClient", "ETHGW:" IPSTR, IP2STR(&event.ip_info.gw)); });

    ec.registerDisconnectedCallback([](){ ESP_LOGI(">>>> EthernetClient", "Disconnected"); });

    ec.init(ETH_RESET, ETH_MDC, ETH_MDIO, ETH_CLK, true);
*/

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include <functional>
#include <esp_log.h>
#include <esp_event.h>
#include <esp_eth.h>
#include <esp_netif.h>
#include <esp_system.h>
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "nvs_flash.h"

using EthernetConnectedCallback = std::function<void(const ip_event_got_ip_t &)>;
using EthernetDisconnectedCallback = std::function<void()>;

class EthernetClient
{
public:
    static constexpr const char *LOG_TAG = "EthernetClient";

    EthernetClient()
        : _isConnected(false),
          _isRunning(false),
          _espNetif(nullptr),
          _eth_handle(nullptr),
          _connectionMutex(xSemaphoreCreateMutex())

    {
        if (!_connectionMutex)
        {
            ESP_LOGE(LOG_TAG, "Failed to create mutex");
        }
    }

    ~EthernetClient()
    {
        if (_connectionMutex)
        {
            vSemaphoreDelete(_connectionMutex);
            _connectionMutex = nullptr;
        }
    }

    bool init(gpio_num_t reset, gpio_num_t mdc, gpio_num_t mdio, gpio_num_t clkout, bool coreInit = false)
    {
        if (!_connectionMutex)
        {
            ESP_LOGE(LOG_TAG, "Mutex is not initialized, cannot proceed");
            return false;
        }

        if (xSemaphoreTake(_connectionMutex, portMAX_DELAY) == pdTRUE)
        {
            if (_eth_handle != nullptr)
            {
                ESP_LOGE(LOG_TAG, "Ethernet is already initialized");
                xSemaphoreGive(_connectionMutex);
                return false;
            }

            xSemaphoreGive(_connectionMutex);
        }

        esp_err_t ret = ESP_FAIL;

        if (coreInit)
        {
            ret = nvs_flash_init();
            if (ret != ESP_OK)
            {
                ESP_LOGE(LOG_TAG, "nvs_flash_init failed: %s", esp_err_to_name(ret));
                return false;
            }

            esp_netif_init();
            ret = esp_event_loop_create_default();
            if (ret != ESP_OK)
            {
                ESP_LOGE(LOG_TAG, "esp_event_loop_create_default failed: %s", esp_err_to_name(ret));
                return false;
            }
        }

        if (reset != gpio_num_t::GPIO_NUM_NC)
        {
            gpio_reset_pin(reset);
            gpio_set_direction(reset, GPIO_MODE_OUTPUT);
        }

        esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();       
        _espNetif = esp_netif_new(&cfg);
        if (!_espNetif)
        {
            ESP_LOGE(LOG_TAG, "esp_netif_new failed: %s", esp_err_to_name(ret));
            return false;
        }

        // #TODO 
        esp_netif_set_default_netif(_espNetif);

        eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
        eth_esp32_emac_config_t esp32_mac_config = {
            .smi_gpio = {.mdc_num = mdc, .mdio_num = mdio},
            .interface = EMAC_DATA_INTERFACE_RMII,
            .clock_config = {.rmii = {.clock_mode = EMAC_CLK_OUT, .clock_gpio = (emac_rmii_clock_gpio_t)clkout}},
            .dma_burst_len = ETH_DMA_BURST_LEN_32,
            .intr_priority = 0};

        eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
        phy_config.phy_addr = 0;
        phy_config.reset_gpio_num = reset;

        gpio_set_level(reset, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(reset, 1);

        esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&esp32_mac_config, &mac_config);
        esp_eth_phy_t *phy = esp_eth_phy_new_lan87xx(&phy_config);

        esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);

        ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config, &_eth_handle));
        esp_netif_attach(_espNetif, esp_eth_new_netif_glue(_eth_handle));

        ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &EthernetClient::ethEventHandler, this));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &EthernetClient::ipEventHandler, this));

        return start();
    }

    bool start()
    {
        if (!_eth_handle)
        {
            ESP_LOGE(LOG_TAG, "Ethernet is not initialized, cannot start");
            return false;
        }

        if (xSemaphoreTake(_connectionMutex, portMAX_DELAY) == pdTRUE)
        {
            if (_isRunning)
            {
                ESP_LOGW(LOG_TAG, "Ethernet is already running");
                xSemaphoreGive(_connectionMutex);
                return true;
            }

            esp_err_t ret = esp_eth_start(_eth_handle);
            _isRunning = (ret == ESP_OK);

            ESP_LOGI(LOG_TAG, "Ethernet %s", _isRunning ? "started successfully" : "failed to start");
            xSemaphoreGive(_connectionMutex);

            return _isRunning;
        }
        return false;
    }

    bool stop()
    {
        if (!_eth_handle)
        {
            ESP_LOGE(LOG_TAG, "Ethernet is not initialized, cannot stop");
            return false;
        }

        if (xSemaphoreTake(_connectionMutex, portMAX_DELAY) == pdTRUE)
        {
            if (!_isRunning)
            {
                ESP_LOGW(LOG_TAG, "Ethernet is not running");
                xSemaphoreGive(_connectionMutex);
                return true;
            }

            esp_err_t ret = esp_eth_stop(_eth_handle);
            _isRunning = (ret != ESP_OK);

            ESP_LOGI(LOG_TAG, "Ethernet %s", !_isRunning ? "stopped successfully" : "failed to stop");
            xSemaphoreGive(_connectionMutex);

            return !_isRunning;
        }
        return false;
    }

    static void ethEventHandler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
    {
        EthernetClient *client = static_cast<EthernetClient *>(arg);

        uint8_t mac_addr[6] = {0};
        /* we can get the ethernet driver handle from event data */
        esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

        switch (event_id)
        {
        case ETHERNET_EVENT_CONNECTED:
            esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
            ESP_LOGI(LOG_TAG, "Ethernet Link Up");
            ESP_LOGI(LOG_TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                     mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

            break;
        case ETHERNET_EVENT_DISCONNECTED:
            ESP_LOGI(LOG_TAG, "Ethernet Link Down");
            if (xSemaphoreTake(client->_connectionMutex, portMAX_DELAY) == pdTRUE)
            {
                client->_isConnected = false;
                xSemaphoreGive(client->_connectionMutex);
            }

            if (client->_disconnectedCallback)
            {
                client->_disconnectedCallback();
            }
            break;
        case ETHERNET_EVENT_START:
            ESP_LOGI(LOG_TAG, "Ethernet Started");
            break;
        case ETHERNET_EVENT_STOP:
            ESP_LOGI(LOG_TAG, "Ethernet Stopped");
            break;
        default:
            break;
        }
    }

    static void ipEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {
        EthernetClient *client = static_cast<EthernetClient *>(arg);
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

        if (event_base == IP_EVENT && event_id == IP_EVENT_ETH_GOT_IP && event)
        {
            const esp_netif_ip_info_t *ip_info = &event->ip_info;
            if (ip_info)
            {
                if (xSemaphoreTake(client->_connectionMutex, portMAX_DELAY) == pdTRUE)
                {
                    client->_isConnected = true;
                    xSemaphoreGive(client->_connectionMutex);
                }

                ESP_LOGI(LOG_TAG, "Ethernet Got IP Address");
                if (client->_connectedCallback)
                {
                    client->_connectedCallback(*event);
                }
            }
        }
    }

    void registerConnectedCallback(EthernetConnectedCallback callback)
    {
        _connectedCallback = callback;
    }

    void registerDisconnectedCallback(EthernetDisconnectedCallback callback)
    {
        _disconnectedCallback = callback;
    }

    bool isConnected() const
    {
        bool connected = false;
        if (xSemaphoreTake(_connectionMutex, portMAX_DELAY) == pdTRUE)
        {
            connected = _isConnected;
            xSemaphoreGive(_connectionMutex);
        }

        return connected;
    }

    esp_netif_t* getNetif() const {
        return _espNetif;
    }

private:
    bool _isConnected;
    bool _isRunning{false};
    esp_netif_t *_espNetif{nullptr};
    esp_eth_handle_t _eth_handle{nullptr};
    EthernetConnectedCallback _connectedCallback{};
    EthernetDisconnectedCallback _disconnectedCallback{};
    SemaphoreHandle_t _connectionMutex;
};
