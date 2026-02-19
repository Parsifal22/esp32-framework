#include "HttpClient.hpp"

const char* HttpClient::TAG = "HTTP_CLIENT";

HttpClient::HttpClient() {}
HttpClient::~HttpClient() {}

esp_err_t HttpClient::_http_event_handler(esp_http_client_event_t *evt) {
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_DATA:
            if (!esp_http_client_is_chunked_response(evt->client)) {
                std::string *response = static_cast<std::string*>(evt->user_data);
                response->append((char*)evt->data, evt->data_len);
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

std::string HttpClient::get(const std::string& url) {
    std::string response_buffer;

    esp_http_client_config_t config = {};
    config.url = url.c_str();
    config.event_handler = _http_event_handler;
    config.user_data = &response_buffer;
    config.method = HTTP_METHOD_GET;
    config.crt_bundle_attach = esp_crt_bundle_attach; 
    config.transport_type = HTTP_TRANSPORT_OVER_SSL;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTPS Status = %d", esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(TAG, "Error: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return response_buffer;
}

std::string HttpClient::post(const std::string& url, const std::string& post_data) {
    std::string response_buffer;

    esp_http_client_config_t config = {};
    config.url = url.c_str();
    config.event_handler = _http_event_handler;
    config.user_data = &response_buffer;
    config.method = HTTP_METHOD_POST;
    config.crt_bundle_attach = esp_crt_bundle_attach;
    config.transport_type = HTTP_TRANSPORT_OVER_SSL;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    esp_http_client_set_post_field(client, post_data.c_str(), post_data.length());
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_err_t err = esp_http_client_perform(client);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return response_buffer;
}

esp_err_t HttpClient::sendTelegramMessage(const std::string& token, const std::string& chat_id, const std::string& text) {
    std::string url = "https://api.telegram.org/bot" + token + "/sendMessage";
    
    std::string payload = "{\"chat_id\": \"" + chat_id + "\", \"text\": \"" + text + "\"}";

    esp_http_client_config_t config = {};
    config.url = url.c_str();
    config.method = HTTP_METHOD_POST;
    config.crt_bundle_attach = esp_crt_bundle_attach; 
    config.transport_type = HTTP_TRANSPORT_OVER_SSL;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    esp_http_client_set_post_field(client, payload.c_str(), payload.length());
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "Telegram sent! Status: %d", status);
    } else {
        ESP_LOGE(TAG, "Telegram failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}