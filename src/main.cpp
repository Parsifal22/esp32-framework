#include "config.hpp"
#include "driver/gpio.h"
// #include "WiFiManager/WiFiManager.hpp"
// #include "Mqtt_Connection/Mqtt_Connection.hpp"
// #include "GPIO/GPIO.hpp"

#define BLINK_GPIO  GPIO_NUM_2 

static const char *TAG = "Blink_Task";

// 1. Функция задачи для мигания светодиодом
void vTaskBlink(void *pvParameters) {
    // Настройка пина
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(500)); // Пауза 500 мс
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // Печатаем, на каком ядре выполняется задача
        ESP_LOGI(TAG, "Blink task running on core %d", xPortGetCoreID());
    }
}

void vTaskSensorSimulation(void *pvParameters) {
    while (1) {
        ESP_LOGW(TAG, "Reading 'sensor' data... (Core %d)", xPortGetCoreID());
        // Имитируем работу
        vTaskDelay(pdMS_TO_TICKS(2000)); 
    }
}

extern "C" void app_main(void) {

    xTaskCreatePinnedToCore(
        vTaskBlink,             // Функция задачи
        "Blink_Task",           // Имя задачи (для отладки)
        2048,                   // Размер стека в байтах
        NULL,                   // Параметры задачи
        2,                      // Приоритет (чем выше число, тем важнее)
        NULL,                   // Хендл задачи
        0                       // Ядро (Core 0)
    );

    // Создаем вторую задачу на ядре 1
    xTaskCreatePinnedToCore(
        vTaskSensorSimulation, 
        "Sensor_Task", 
        2048, 
        NULL, 
        1,                      // Приоритет ниже, чем у мигалки
        NULL, 
        1                       // Ядро (Core 1)
    );
    // GPIO joystick_button(GPIO_NUM_14, GPIO_MODE_INPUT);
    // GPIO joystick_x(ADC_CHANNEL_6);
    // GPIO joystick_y(ADC_CHANNEL_4);

    // WiFiManager wifi("Nothing", "Nothing654");

    // if (wifi.connect() == ESP_OK) {
    //     printf("We connected\n");

    //     Mqtt_Connection mqtt;
    //     mqtt.begin("mqtt://broker.emqx.io");

    //     vTaskDelay(pdMS_TO_TICKS(2000));
    //     while (true) {
    //          mqtt.publish("school/test/sensor", "{\"temp\": 25.4, \"status\": \"OK\"}");
    //      }
    // }

    // while (true) {
    //     vTaskDelay(pdMS_TO_TICKS(5000));
    //     printf("Joystick X: %d, Y: %d, Button: %d\n", joystick_x.get_level(), joystick_y.get_level(), joystick_button.get_level());
    // }

}