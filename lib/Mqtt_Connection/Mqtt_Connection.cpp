#include "Mqtt_Connection.hpp"

const char* Mqtt_Connection::TAG = "MQTT_CLASS";

void Mqtt_Connection::mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {

    auto event = (esp_mqtt_event_handle_t)event_data;
    
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Successfully connected to MQTT broker.");
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "Connection lost. Automatic reconnection attempt will be made.");
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "An MQTT error occurred");
            break;
        case MQTT_EVENT_DATA:
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        default:
            break;
    }
}


void Mqtt_Connection::begin(const std::string& broker_url) {

    esp_mqtt_client_config_t mqtt_cfg = {};
    

    mqtt_cfg.broker.address.uri = broker_url.c_str();


    client = esp_mqtt_client_init(&mqtt_cfg);


    esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, this);
    
    esp_mqtt_client_start(client);
}


void Mqtt_Connection::publish(const std::string& topic, const std::string& data) {
    if (client) {

        int msg_id = esp_mqtt_client_publish(client, topic.c_str(), data.c_str(), 0, 1, 0);
        
        if (msg_id != -1) {
            ESP_LOGI(TAG, "Data send successfully. Data ID %d", msg_id);
        } else {
            ESP_LOGE(TAG, "Error with sending the message!");
        }
    } else {
        ESP_LOGE(TAG, "Attempted to publish before client initialization!");
    }
}

void Mqtt_Connection::subscribe(const std::string& topic, int qos) {
    if (client) {
        int msg_id = esp_mqtt_client_subscribe(client, topic.c_str(), qos);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    }
}