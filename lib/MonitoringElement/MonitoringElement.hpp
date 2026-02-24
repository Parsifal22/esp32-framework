#ifndef MONITORING_ELEMENT_HPP
#define MONITORING_ELEMENT_HPP
#include "esp_private/esp_clk.h"
extern "C" {
    #include "esp_pm.h"
    #include "esp_sleep.h"
    #include "portmacro.h"
}

enum class PowerMode {
    Active,     
    ModemSleep, 
    LightSleep, 
    DeepSleep,
    Hibernation   
};

class MonitoringElement {
public:
    static MonitoringElement& getInstance();
    int getControllerFrequency() const;

    PowerMode getCurrentPowerMode() const;
    void setPowerMode(PowerMode mode);

private:
    MonitoringElement();
    MonitoringElement(const MonitoringElement&) = delete;
    MonitoringElement& operator=(const MonitoringElement&) = delete;

    PowerMode current_mode_ = PowerMode::Active;
    
    void configurePowerManagement(PowerMode mode);

    void enterHibernation();
};

#endif
