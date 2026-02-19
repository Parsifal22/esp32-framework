#ifndef GPIO_HPP
#define GPIO_HPP
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

class GPIO {
public:

    GPIO(gpio_num_t pin, gpio_mode_t mode);

    GPIO(adc_channel_t channel);

    void set_level(uint32_t level);

    int get_level();

    gpio_mode_t get_mode();

    gpio_num_t get_pin();

    void enable_interrupt(QueueHandle_t queue);

private:
    gpio_num_t _pin;
    gpio_mode_t _mode;

    adc_channel_t _adc_chan;
    adc_oneshot_unit_handle_t _adc_handle;
    bool _is_analog = false;

    QueueHandle_t _target_queue;

    static void IRAM_ATTR gpio_isr_handler(void* arg);
};

#endif 