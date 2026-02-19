#include "WiFiManager.hpp"
#include "Mqtt_Connection.hpp"
#include "GPIO.hpp"
#include "examples.hpp"

void Wi_Fi_connection_example(void) {
    GPIO joystick_button(GPIO_NUM_14, GPIO_MODE_INPUT);
    GPIO joystick_x(ADC_CHANNEL_6);
    GPIO joystick_y(ADC_CHANNEL_4);

    WiFiManager wifi("SSID", "PASSWORD");

    if (wifi.connect() == ESP_OK) {
        printf("We connected\n");

        Mqtt_Connection mqtt;
        mqtt.begin("mqtt://broker.emqx.io");

        vTaskDelay(pdMS_TO_TICKS(2000));
        while (true) {
             mqtt.publish("school/test/sensor", "{\"temp\": 25.4, \"status\": \"OK\"}");
         }
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        printf("Joystick X: %d, Y: %d, Button: %d\n", joystick_x.get_level(), joystick_y.get_level(), joystick_button.get_level());
    }

}
