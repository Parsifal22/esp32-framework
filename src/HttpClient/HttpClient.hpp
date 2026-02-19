#pragma once

#include "../config.hpp"
#include <vector>
#include "esp_http_client.h"


class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    std::string get(const std::string& url);

    std::string post(const std::string& url, const std::string& post_data);

    esp_err_t sendTelegramMessage(const std::string& token, const std::string& chat_id, const std::string& text);

private:
    static esp_err_t _http_event_handler(esp_http_client_event_t *evt);
    static const char* TAG;
};