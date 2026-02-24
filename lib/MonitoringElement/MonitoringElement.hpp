#ifndef MONITORING_ELEMENT_HPP
#define MONITORING_ELEMENT_HPP
#include "esp_private/esp_clk.h"
extern "C" {
    #include "esp_pm.h"
    #include "esp_sleep.h"
    #include "portmacro.h"
}

class MonitoringElement {
public:
    static MonitoringElement& getInstance();
    int getControllerFrequency() const;

private:
    MonitoringElement();
    MonitoringElement(const MonitoringElement&) = delete;
    MonitoringElement& operator=(const MonitoringElement&) = delete;
};

#endif
