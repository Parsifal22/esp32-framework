#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "esp_wifi.h"      
#include "esp_event.h"     
#ifdef __cplusplus
#include <string>
#endif
#include "esp_log.h" 

#pragma once

#include <string>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

class WiFiManager {
public:
    WiFiManager(const std::string& ssid, const std::string& password);
    ~WiFiManager();

    esp_err_t init();
    esp_err_t connect();
    void disconnect();

private:
    static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                 int32_t event_id, void* event_data);

    std::string _ssid;
    std::string _password;
    int _retry_count = 0;
    const int MAX_RETRY = 5;

    EventGroupHandle_t _wifi_event_group;
    esp_event_handler_instance_t _instance_any_id;
    esp_event_handler_instance_t _instance_got_ip;
    esp_netif_t* _netif = nullptr;

    static const int WIFI_CONNECTED_BIT = BIT0;
    static const int WIFI_FAIL_BIT      = BIT1;
};