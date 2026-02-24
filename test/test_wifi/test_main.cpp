#include <unity.h>
#include "WiFiManager.hpp"
#include "esp_log.h"


#define TEST_SSID "Nothing"
#define WIFI_TEST_PASS "Nothing654"

void test_wifi_connection_success(void) {
    WiFiManager wifi(TEST_SSID, WIFI_TEST_PASS);

    esp_err_t result = wifi.connect();
    TEST_ASSERT_EQUAL(ESP_OK, result);
}

void test_wifi_connection_wrong_password(void) {

    WiFiManager wifi("CorrectSSID", "WRONG_PASSWORD");
    
    esp_err_t result = wifi.connect();
    

    TEST_ASSERT_EQUAL(ESP_FAIL, result);
}

void test_wifi_reconnection_on_disconnect(void) {
    WiFiManager wifi(TEST_SSID, WIFI_TEST_PASS);
    

    TEST_ASSERT_EQUAL(ESP_OK, wifi.init());
    TEST_ASSERT_EQUAL(ESP_OK, wifi.connect());
    ESP_LOGI("TEST", "Шаг 1: Соединение установлено");


    ESP_LOGI("TEST", "Шаг 2: Имитируем обрыв связи...");
    
   
    wifi_event_sta_disconnected_t disconnect_reason;
    disconnect_reason.reason = WIFI_REASON_BEACON_TIMEOUT; 
    
    ESP_ERROR_CHECK(esp_event_post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, 
                                  &disconnect_reason, sizeof(disconnect_reason), 
                                  portMAX_DELAY));

    
    vTaskDelay(pdMS_TO_TICKS(5000)); 

    TEST_ASSERT_EQUAL(ESP_OK, wifi.connect());
    ESP_LOGI("TEST", "Шаг 3: Соединение успешно восстановлено автоматически");
}

extern "C" void app_main() {

    vTaskDelay(pdMS_TO_TICKS(2000));

    UNITY_BEGIN();
    

    RUN_TEST(test_wifi_connection_success);
    

    RUN_TEST(test_wifi_connection_wrong_password);

    RUN_TEST(test_wifi_reconnection_on_disconnect);

    UNITY_END();
}