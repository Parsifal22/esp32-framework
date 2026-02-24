#include "MonitoringElement.hpp"
#include "esp_log.h"

static const char* TAG = "MON_EL";

MonitoringElement::MonitoringElement() : current_mode_(PowerMode::Active) {
    setPowerMode(PowerMode::Active);
}

MonitoringElement& MonitoringElement::getInstance() {
    static MonitoringElement instance;
    return instance;
}

int MonitoringElement::getControllerFrequency() const {
    return esp_clk_cpu_freq();
}

PowerMode MonitoringElement::getCurrentPowerMode() const {
    return current_mode_;
}

void MonitoringElement::setPowerMode(PowerMode mode) {
    current_mode_ = mode;
    
    switch (mode) {
        case PowerMode::Active:
            ESP_LOGI(TAG, "Switching to Active Mode");
            configurePowerManagement(PowerMode::Active);
            break;

        case PowerMode::ModemSleep:
            ESP_LOGI(TAG, "Switching to Modem-sleep");
            configurePowerManagement(PowerMode::ModemSleep);
            break;

        case PowerMode::LightSleep:
            ESP_LOGI(TAG, "Entering Light-sleep...");
            esp_light_sleep_start();
            break;

        case PowerMode::DeepSleep:
            ESP_LOGI(TAG, "Entering Deep-sleep. Goodbye!");
            esp_deep_sleep_start();
            break;

        case PowerMode::Hibernation:
            enterHibernation();
            break;
    }
}

void MonitoringElement::configurePowerManagement(PowerMode mode) {
    esp_pm_config_t pm_config;
    
    if (mode == PowerMode::Active) {
        pm_config.max_freq_mhz = 160; 
        pm_config.min_freq_mhz = 160;
        pm_config.light_sleep_enable = false;
    } else {
        pm_config.max_freq_mhz = 160;
        pm_config.min_freq_mhz = 40;
        pm_config.light_sleep_enable = (mode == PowerMode::LightSleep);
    }

    esp_err_t err = esp_pm_configure(&pm_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure Power Management: %s", esp_err_to_name(err));
    }
}


void MonitoringElement::enterHibernation() {
    ESP_LOGI(TAG, "Entering Hibernation Mode...");

    esp_sleep_pd_config(ESP_PD_DOMAIN_RC_FAST, ESP_PD_OPTION_OFF);
    
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);

    esp_sleep_enable_timer_wakeup(20 * 1000000);
    esp_deep_sleep_start();
}