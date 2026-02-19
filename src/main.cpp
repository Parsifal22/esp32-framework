#include "Examples/examples.hpp"
#include "HttpClient.hpp"
#include "WiFiManager.hpp"

void telegram_task(void *pvParameters) {
    HttpClient http;
    std::string token = "Token";
    std::string chat_id = "chat`id";
    
    http.sendTelegramMessage(token, chat_id, "ESP32!");
    
    vTaskDelete(NULL); 
}

extern "C" void app_main(void) {
    WiFiManager wifi("SSID", "Paaword");
    
    if (wifi.connect() == ESP_OK) {
        HttpClient http;
        
        std::string response = http.get("https://jsonplaceholder.typicode.com/posts/1");
        printf("Response: %s\n", response.c_str());

        std::string result = http.post("https://jsonplaceholder.typicode.com/posts", "{\"status\":\"ok\"}");

        printf("POST Response: %s\n", result.c_str());
        xTaskCreate(telegram_task, "telegram_task", 8192, NULL, 5, NULL);
    }
}