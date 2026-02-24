#ifndef GPIO_HPP
#define GPIO_HPP
#include <unordered_map>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include <string>

class GPIO {
public:

    GPIO(gpio_num_t pin, gpio_mode_t mode, std::string name);

    GPIO(adc_channel_t channel, std::string name);

    void set_level(uint32_t level);

    int get_level();

    gpio_mode_t get_mode();

    gpio_num_t get_pin();

    void enable_interrupt(QueueHandle_t queue);

    static GPIO* get_by_name(const std::string& name);
    static const std::unordered_map<std::string, GPIO*>& get_all();

private:
    gpio_num_t _pin;
    gpio_mode_t _mode;

    adc_channel_t _adc_chan;
    adc_oneshot_unit_handle_t _adc_handle;
    bool _is_analog = false;

    QueueHandle_t _target_queue;

    void register_gpio(const std::string& name, GPIO* gpio);

    static void IRAM_ATTR gpio_isr_handler(void* arg);
    static std::unordered_map<std::string, GPIO*> _registry;
};

#endif 