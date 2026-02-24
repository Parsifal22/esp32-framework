
#include "MonitoringElement.hpp"
#include "freertos/FreeRTOS.h"

extern "C" void app_main(void) {
    MonitoringElement& monitor = MonitoringElement::getInstance();
    while (true) {
        int freq = monitor.getControllerFrequency();
        printf("Controller Frequency: %d MHz\n", freq);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

}