#include "WiFiManager.hpp"
#include <cstring>

static const char* TAG = "WiFiManager";

WiFiManager::WiFiManager(const std::string& ssid, const std::string& password)
    : _ssid(ssid), _password(password) {
    _wifi_event_group = xEventGroupCreate();
}

WiFiManager::~WiFiManager() {
    vEventGroupDelete(_wifi_event_group);
}

esp_err_t WiFiManager::init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    if (esp_netif_init() != ESP_OK) {
        ESP_LOGW(TAG, "Netif already initialized");
    }

    if (esp_event_loop_create_default() != ESP_OK) {
        ESP_LOGW(TAG, "Event loop already initialized");
    }

    _netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                    &WiFiManager::wifi_event_handler, this, &_instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                    &WiFiManager::wifi_event_handler, this, &_instance_got_ip));

    return ESP_OK;
}

esp_err_t WiFiManager::connect() {
    xEventGroupClearBits(_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
    _retry_count = 0;

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid, _ssid.c_str(), sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, _password.c_str(), sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    
    ESP_LOGI(TAG, "Starting WiFi...");
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        return ESP_OK;
    }
    return ESP_FAIL;
}

void WiFiManager::wifi_event_handler(void* arg, esp_event_base_t event_base,
                                   int32_t event_id, void* event_data) {
    WiFiManager* obj = static_cast<WiFiManager*>(arg);

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (obj->_retry_count < obj->MAX_RETRY) {
            esp_wifi_connect();
            obj->_retry_count++;
            ESP_LOGI(TAG, "Retrying connection... (%d/%d)", obj->_retry_count, obj->MAX_RETRY);
        } else {
            xEventGroupSetBits(obj->_wifi_event_group, WIFI_FAIL_BIT);
        }
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        obj->_retry_count = 0;
        xEventGroupSetBits(obj->_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}