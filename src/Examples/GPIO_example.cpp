#include "examples.hpp"

#define BLINK_GPIO    GPIO_NUM_2 
#define STATUS_GPIO   GPIO_NUM_18  
#define TRIG_GPIO     GPIO_NUM_12
#define ECHO_GPIO     GPIO_NUM_14
#define BUTTON_GPIO   GPIO_NUM_4

static const char *TAG = "RTOS_EVENT_GROUP";


#define SENSOR_RUNNING_BIT (1 << 0) 

typedef struct {
    float distance;
    uint32_t timestamp;
} sensor_data_t;

sensor_data_t g_latest_sensor_data = { .distance = 100.0f, .timestamp = 0 };
bool is_measuring = false;

SemaphoreHandle_t xDistanceMutex;
SemaphoreHandle_t xButtonSemaphore;
EventGroupHandle_t xSystemEventGroup; 

void IRAM_ATTR button_isr_handler(void* arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xButtonSemaphore, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

void vTaskStatusLED(void *pvParameters) {
    gpio_reset_pin(STATUS_GPIO);
    gpio_set_direction(STATUS_GPIO, GPIO_MODE_OUTPUT);

    while (1) {

        EventBits_t bits = xEventGroupWaitBits(xSystemEventGroup, SENSOR_RUNNING_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

        if (bits & SENSOR_RUNNING_BIT) {
            gpio_set_level(STATUS_GPIO, 1);
            ESP_LOGW(TAG, "Status ON");
        } else {
            gpio_set_level(STATUS_GPIO, 0);
            ESP_LOGW(TAG, "Status OFF");
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vTaskBlink(void *pvParameters) {
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    int delay_ms = 500;

    while (1) {
        if (xSemaphoreTake(xDistanceMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            float dist = g_latest_sensor_data.distance;
            xSemaphoreGive(xDistanceMutex);

            if (dist < 10.0f)      delay_ms = 100;
            else if (dist < 30.0f) delay_ms = 250;
            else                   delay_ms = 800;
        }

        int final_delay = is_measuring ? delay_ms : 1000;
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(final_delay));
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(final_delay));
    }
}

void vTaskUltrasonic(void *pvParameters) {
    gpio_reset_pin(TRIG_GPIO);
    gpio_set_direction(TRIG_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(ECHO_GPIO);
    gpio_set_direction(ECHO_GPIO, GPIO_MODE_INPUT);

    while (1) {
        if (xSemaphoreTake(xButtonSemaphore, 0) == pdTRUE) {
            is_measuring = !is_measuring;
            
            if (is_measuring) {
                ESP_LOGW(TAG, "Sensor START");
                xEventGroupSetBits(xSystemEventGroup, SENSOR_RUNNING_BIT);
            } else {
                ESP_LOGW(TAG, "Sensor STOP");
                xEventGroupClearBits(xSystemEventGroup, SENSOR_RUNNING_BIT);
            }
            vTaskDelay(pdMS_TO_TICKS(300));
        }

        if (is_measuring) {
            gpio_set_level(TRIG_GPIO, 0);
            esp_rom_delay_us(2);
            gpio_set_level(TRIG_GPIO, 1);
            esp_rom_delay_us(10);
            gpio_set_level(TRIG_GPIO, 0);

            uint32_t start = esp_timer_get_time();
            while (gpio_get_level(ECHO_GPIO) == 0 && (esp_timer_get_time() - start < 100000));
            uint32_t echo_start = esp_timer_get_time();
            while (gpio_get_level(ECHO_GPIO) == 1 && (esp_timer_get_time() - echo_start < 100000));
            uint32_t echo_end = esp_timer_get_time();

            sensor_data_t newData;
            newData.distance = ((float)(echo_end - echo_start) * 0.0343) / 2;
            newData.timestamp = (uint32_t)(esp_timer_get_time() / 1000);

            if (xSemaphoreTake(xDistanceMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                g_latest_sensor_data = newData;
                xSemaphoreGive(xDistanceMutex);
            }
            vTaskDelay(pdMS_TO_TICKS(200));
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void GPIO_example(void) {

    xButtonSemaphore = xSemaphoreCreateBinary();
    xDistanceMutex = xSemaphoreCreateMutex();
    xSystemEventGroup = xEventGroupCreate();

    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&btn_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL);

    xTaskCreatePinnedToCore(vTaskBlink, "Blink", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(vTaskUltrasonic, "Ultra", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(vTaskStatusLED, "StatusLED", 2048, NULL, 1, NULL, 0);
}