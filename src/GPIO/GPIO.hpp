#ifndef GPIO_HPP
#define GPIO_HPP
#include "../config.hpp"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

class GPIO {
public:
    GPIO(gpio_num_t pin, gpio_mode_t mode) : _pin(pin), _mode(mode) {
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE; // Disable interrupts by default
        io_conf.mode = mode;
        io_conf.pin_bit_mask = (1ULL << _pin);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        gpio_config(&io_conf);
    }

    GPIO(adc_channel_t channel) : _adc_chan(channel), _is_analog(true) {
        
        adc_oneshot_unit_init_cfg_t init_config = {
            .unit_id = ADC_UNIT_1,
            .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
            .ulp_mode = ADC_ULP_MODE_DISABLE,
        };
        adc_oneshot_new_unit(&init_config, &_adc_handle);

        adc_oneshot_chan_cfg_t config = {
            .atten = ADC_ATTEN_DB_12, 
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        adc_oneshot_config_channel(_adc_handle, _adc_chan, &config);
    }

    void set_level(uint32_t level) {
        if (!_is_analog) gpio_set_level(_pin, level);
    }

    int get_level() {
        if (_is_analog) {
            int val;
            adc_oneshot_read(_adc_handle, _adc_chan, &val);
            return val;
        } else {
            return gpio_get_level(_pin);
        }
    }

    gpio_mode_t get_mode() {
        return _mode;
    }

    gpio_num_t get_pin() {
        return _pin;
    }

    void enable_interrupt(QueueHandle_t queue) {
        _target_queue = queue;
        gpio_install_isr_service(0); 
        gpio_isr_handler_add(_pin, gpio_isr_handler, (void*) this);
    }

private:
    // Digital Members
    gpio_num_t _pin;
    gpio_mode_t _mode;

    // Analog Members
    adc_channel_t _adc_chan;
    adc_oneshot_unit_handle_t _adc_handle;
    bool _is_analog = false;

    QueueHandle_t _target_queue;

    static void IRAM_ATTR gpio_isr_handler(void* arg) {
        GPIO* obj = (GPIO*) arg;
        int pin_val = gpio_get_level(obj->_pin);
        
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(obj->_target_queue, &pin_val, &xHigherPriorityTaskWoken);
        
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
};

#endif 