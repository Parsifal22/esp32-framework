#include "MonitoringElement.hpp"


MonitoringElement::MonitoringElement() {
    esp_pm_config_esp32c3_t pm_config = {
        .max_freq_mhz = 160, 
        .min_freq_mhz = 10,  
        .light_sleep_enable = true
    };

    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
}

MonitoringElement& MonitoringElement::getInstance() {
    static MonitoringElement instance;
    return instance;
}

int MonitoringElement::getControllerFrequency() const {
    return esp_clk_cpu_freq();
}
