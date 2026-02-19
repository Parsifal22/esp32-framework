# Libraries Documentation

This directory contains reusable libraries for the ESP32 IoT project. Each library is self-contained with its own implementation files, headers, and metadata.

## Overview

The `lib/` folder follows **PlatformIO's library structure**. Each subdirectory is an independent library that can be:
- Compiled as part of the project
- Reused in other ESP32 projects
- Distributed as a standalone package

All libraries include:
- `.hpp` header files (class declarations)
- `.cpp` implementation files (method implementations)
- `library.json` metadata file (PlatformIO discovery)

### Key FreeRTOS Note

All libraries in this project include FreeRTOS headers. **Important**: Always include `freertos/FreeRTOS.h` **first** before any other FreeRTOS headers (like `queue.h`, `task.h`, `event_groups.h`).

---

## 1. GPIO Library

**Location**: `lib/GPIO/`

**Purpose**: Abstraction layer for digital and analog GPIO control on ESP32.

### Features

- **Digital GPIO**: Standard GPIO input/output operations
- **Analog GPIO (ADC)**: Support for analog-to-digital conversion
- **Interrupts**: Hardware interrupt support with ISR handlers
- **Queue Integration**: Send interrupt events to FreeRTOS queues

### Files

| File | Purpose |
|------|---------|
| `GPIO.hpp` | Class declaration with two constructors |
| `GPIO.cpp` | Implementation of digital/analog init, read, write, and ISR |
| `library.json` | PlatformIO metadata |

### Class Declaration

```cpp
class GPIO {
public:
    // Digital GPIO constructor
    GPIO(gpio_num_t pin, gpio_mode_t mode);
    
    // Analog GPIO (ADC) constructor
    GPIO(adc_channel_t channel);
    
    void set_level(uint32_t level);      // Set digital output
    int get_level();                      // Read digital or analog input
    gpio_mode_t get_mode();               // Get GPIO mode
    gpio_num_t get_pin();                 // Get pin number
    void enable_interrupt(QueueHandle_t queue); // Enable ISR with queue
};
```

### Usage Examples

#### Digital GPIO (Output - LED)

```cpp
#include "GPIO.hpp"

GPIO led(GPIO_NUM_2, GPIO_MODE_OUTPUT);
led.set_level(1);  // Turn on
led.set_level(0);  // Turn off
```

#### Digital GPIO (Input - Button)

```cpp
GPIO button(GPIO_NUM_0, GPIO_MODE_INPUT);
int state = button.get_level();  // Read button state (0 or 1)
```

#### Analog GPIO (ADC - Sensor)

```cpp
GPIO sensor(ADC_CHANNEL_6);  // ADC1 Channel 6
int adc_value = sensor.get_level();  // Read ADC value (0-4095)
```

#### GPIO with Interrupt

```cpp
QueueHandle_t gpio_queue = xQueueCreate(10, sizeof(int));
GPIO button(GPIO_NUM_0, GPIO_MODE_INPUT);
button.enable_interrupt(gpio_queue);

// In a task:
int pin_val;
if (xQueueReceive(gpio_queue, &pin_val, portMAX_DELAY)) {
    printf("Button state changed: %d\n", pin_val);
}
```

### Implementation Details

#### Digital GPIO Initialization
- Disables interrupts by default
- Disables pull-up/pull-down resistors
- Configures GPIO pin with specified mode
- Uses ESP-IDF gpio_config() API

#### Analog GPIO Initialization
- Creates ADC oneshot unit (ADC_UNIT_1)
- Configures 12dB attenuation (full range: 0-3.3V mapped to 0-4095)
- Uses default bitwidth conversion
- Uses ESP-IDF ADC oneshot API

#### ISR Handler
- Thread-safe queue send from interrupt context
- Automatically yields to higher priority task if needed
- Runs in IRAM for deterministic response

### Key Includes

```cpp
#include "freertos/FreeRTOS.h"  // MUST be first
#include "freertos/queue.h"     // For interrupt queue
#include "driver/gpio.h"        // Digital GPIO driver
#include "esp_adc/adc_oneshot.h" // ADC driver
```

---

## 2. HttpClient Library

**Location**: `lib/HttpClient/`

**Purpose**: HTTP client for making GET/POST requests and sending Telegram messages.

### Features

- **GET Requests**: Retrieve data from HTTP endpoints
- **POST Requests**: Send JSON data to endpoints
- **SSL/TLS Support**: Uses ESP-IDF HTTP client with certificate bundle
- **Telegram Integration**: Direct message sending to Telegram bots
- **Response Buffering**: Accumulates response data during callbacks

### Files

| File | Purpose |
|------|---------|
| `HttpClient.hpp` | Class declaration with GET, POST, and Telegram methods |
| `HttpClient.cpp` | HTTP implementation with event handler |
| `library.json` | PlatformIO metadata |

### Class Declaration

```cpp
class HttpClient {
public:
    HttpClient();
    ~HttpClient();
    
    std::string get(const std::string& url);
    std::string post(const std::string& url, const std::string& post_data);
    esp_err_t sendTelegramMessage(const std::string& token, 
                                   const std::string& chat_id, 
                                   const std::string& text);
};
```

### Usage Examples

#### GET Request

```cpp
#include "HttpClient.hpp"

HttpClient http;
std::string response = http.get("https://api.example.com/data");
printf("Response: %s\n", response.c_str());
```

#### POST Request

```cpp
HttpClient http;
std::string payload = "{\"status\":\"online\",\"temperature\":25.5}";
std::string response = http.post("https://api.example.com/update", payload);
```

#### Send Telegram Message

```cpp
HttpClient http;
esp_err_t result = http.sendTelegramMessage(
    "123456:ABC-DEF1234ghIkl-zyx57W2v1u123ew11",  // Bot token
    "987654321",                                     // Chat ID
    "Hello from ESP32!"
);

if (result == ESP_OK) {
    printf("Message sent successfully\n");
}
```

### Implementation Details

#### HTTP Event Handler
- Handles multiple HTTP events:
  - `HTTP_EVENT_ERROR`: Connection errors
  - `HTTP_EVENT_ON_DATA`: Accumulates response data
  - Other events: Logged but not processed
- Response data appended to string buffer via user_data callback

#### Telegram API Integration
- Uses Telegram Bot API endpoint: `https://api.telegram.org/botTOKEN/sendMessage`
- POST parameters: `chat_id` and `text`
- URL-encodes text for safety

#### SSL/TLS Configuration
- Uses ESP-IDF certificate bundle for trust validation
- Supports modern HTTPS endpoints

### Key Includes

```cpp
#include "esp_http_client.h"   // HTTP client API
#include "esp_crt_bundle.h"    // SSL certificate bundle
#include "esp_log.h"           // Logging
#include <string>              // std::string
```

---

## 3. WiFiManager Library

**Location**: `lib/WiFiManager/`

**Purpose**: WiFi connection management with automatic reconnection and event handling.

### Features

- **WiFi Connection**: Connect to WiFi networks with SSID/password
- **Event Handling**: Manages WiFi connection/disconnection events
- **Auto-Reconnect**: Automatic reconnection with retry limit
- **FreeRTOS Integration**: Uses event groups for synchronization
- **Non-Blocking**: Connection process runs via FreeRTOS events

### Files

| File | Purpose |
|------|---------|
| `WiFiManager.hpp` | Class declaration with connect method |
| `WiFiManager.cpp` | Event handler and initialization |
| `library.json` | PlatformIO metadata |

### Class Declaration

```cpp
class WiFiManager {
public:
    WiFiManager(const std::string& ssid, const std::string& password);
    esp_err_t connect();

private:
    std::string _ssid;
    std::string _password;
    EventGroupHandle_t _wifi_event_group;
    int _retry_count = 0;
    const int MAX_RETRY = 5;
    
    static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                   int32_t event_id, void* event_data);
};
```

### Usage Examples

#### Basic WiFi Connection

```cpp
#include "WiFiManager.hpp"

WiFiManager wifi("MySSID", "MyPassword");

if (wifi.connect() == ESP_OK) {
    printf("Connected to WiFi!\n");
    // Make HTTP requests, connect to MQTT, etc.
} else {
    printf("Failed to connect\n");
}
```

#### Connection in Main Task

```cpp
extern "C" void app_main(void) {
    WiFiManager wifi("SSID", "Password");
    
    if (wifi.connect() == ESP_OK) {
        // WiFi is now ready
        // Can be used by other tasks
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

### Implementation Details

#### Initialization Sequence
1. Initializes NVS (Non-Volatile Storage)
2. Creates WiFi event group for synchronization
3. Initializes WiFi in STA (Station) mode
4. Registers event handlers for WiFi events

#### Event Handling
- **WIFI_EVENT_STA_START**: Initiates connection attempt
- **WIFI_EVENT_STA_DISCONNECTED**: 
  - Increments retry counter
  - Attempts reconnection (up to MAX_RETRY=5)
  - Logs connection attempts
- **IP_EVENT_STA_GOT_IP**: Sets WiFi connected flag

#### Connection Blocking
- `connect()` method blocks until:
  - WiFi is connected and IP obtained
  - Maximum retry attempts reached
  - Returns `ESP_OK` on success, error code otherwise

### Key Includes

```cpp
#include "freertos/FreeRTOS.h"      // MUST be first
#include "freertos/event_groups.h"  // Event synchronization
#include "nvs_flash.h"              // Non-volatile storage
#include "esp_wifi.h"               // WiFi driver
#include "esp_event.h"              // Event loop
#include "esp_log.h"                // Logging
```

---

## 4. Mqtt_Connection Library

**Location**: `lib/Mqtt_Connection/`

**Purpose**: MQTT client for publish/subscribe messaging in IoT applications.

### Features

- **MQTT Broker Connection**: Connect to any MQTT broker (EMQX, Mosquitto, AWS, etc.)
- **Pub/Sub**: Publish messages and subscribe to topics
- **Auto-Reconnect**: Automatic reconnection on connection loss
- **Event Loop Integration**: Uses ESP-IDF event loop for callbacks
- **SSL/TLS Support**: Secure connections with mqtt:// and mqtts://

### Files

| File | Purpose |
|------|---------|
| `Mqtt_Connection.hpp` | Class declaration with begin, publish, subscribe methods |
| `Mqtt_Connection.cpp` | MQTT client initialization and event handling |
| `library.json` | PlatformIO metadata |

### Class Declaration

```cpp
class Mqtt_Connection {
public:
    void begin(const std::string& broker_url);
    esp_err_t publish(const std::string& topic, const std::string& data);
    esp_err_t subscribe(const std::string& topic);

private:
    esp_mqtt_client_handle_t client;
    static const char* TAG;
    static void mqtt_event_handler(void *handler_args, esp_event_base_t base, 
                                   int32_t event_id, void *event_data);
};
```

### Usage Examples

#### Basic MQTT Connection and Publish

```cpp
#include "Mqtt_Connection.hpp"

Mqtt_Connection mqtt;
mqtt.begin("mqtt://broker.emqx.io:1883");

esp_err_t result = mqtt.publish("esp32/sensor/temperature", "25.5");
if (result == ESP_OK) {
    printf("Message published\n");
}
```

#### Publish in a Task

```cpp
void sensor_task(void *pvParameters) {
    Mqtt_Connection *mqtt = static_cast<Mqtt_Connection*>(pvParameters);
    
    while (1) {
        float temperature = read_temperature();
        std::string payload = std::to_string(temperature);
        
        mqtt->publish("home/livingroom/temperature", payload);
        vTaskDelay(pdMS_TO_TICKS(10000));  // Every 10 seconds
    }
}
```

#### Subscribe to Topic

```cpp
Mqtt_Connection mqtt;
mqtt.begin("mqtt://broker.emqx.io");
mqtt.subscribe("home/control/led");  // Will receive messages on this topic
```

### Implementation Details

#### MQTT Event Handler
Handles events:
- **MQTT_EVENT_CONNECTED**: Logs successful connection
- **MQTT_EVENT_DISCONNECTED**: Logs connection loss, triggers auto-reconnect
- **MQTT_EVENT_SUBSCRIBED**: Confirms topic subscription
- **MQTT_EVENT_DATA**: Processes received messages
- **MQTT_EVENT_ERROR**: Reports connection errors

#### Broker Configuration
- Supports any MQTT broker URI format
- `mqtt://` for unencrypted connections
- `mqtts://` for SSL/TLS encrypted connections
- Default port: 1883 (mqtt) or 8883 (mqtts)

#### Message Format
- Topics: `home/livingroom/temperature`, `device/status`, etc.
- Payload: Any string data (JSON, plain text, binary, etc.)

### Key Includes

```cpp
#include "mqtt_client.h"   // MQTT driver
#include "esp_log.h"       // Logging
#include <string>          // std::string
```

---

## Library Integration with PlatformIO

### library.json Structure

Each library contains a `library.json` file:

```json
{
  "name": "GPIO",
  "version": "1.0.0",
  "description": "GPIO library for ESP32",
  "keywords": "gpio, digital, analog, adc, esp32",
  "frameworks": ["espidf"],
  "platforms": ["espressif32"]
}
```

This file helps PlatformIO:
- Discover and recognize the library
- Include correct header paths automatically
- Manage dependencies
- Support library portability

### Including Libraries in Your Code

Once in `lib/`, you can include headers directly:

```cpp
// These work automatically with PlatformIO
#include "GPIO.hpp"
#include "HttpClient.hpp"
#include "WiFiManager.hpp"
#include "Mqtt_Connection.hpp"
```

No need for relative paths like `"../lib/GPIO/GPIO.hpp"`.

---

## Common Patterns

### FreeRTOS Task with Multiple Libraries

```cpp
void iot_task(void *pvParameters) {
    WiFiManager wifi("SSID", "Password");
    if (wifi.connect() != ESP_OK) return;
    
    Mqtt_Connection mqtt;
    mqtt.begin("mqtt://broker.emqx.io");
    
    HttpClient http;
    std::string weather = http.get("https://api.weather.api/data");
    
    GPIO sensor(ADC_CHANNEL_6);
    
    while (1) {
        int adc_value = sensor.get_level();
        std::string payload = "sensor:" + std::to_string(adc_value);
        mqtt->publish("home/sensor", payload);
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
```

### Error Handling

```cpp
esp_err_t result = mqtt.publish("topic", "data");
if (result != ESP_OK) {
    ESP_LOGE("MAIN", "MQTT publish failed: %s", esp_err_to_name(result));
}
```

---

## Extending the Libraries

To add a new method to a library:

1. **Add declaration** in `.hpp` file
2. **Add implementation** in `.cpp` file
3. **Update library.json** version if needed
4. **Rebuild** with `platformio run`

Example: Adding WiFi scan to WiFiManager

```cpp
// WiFiManager.hpp
std::vector<std::string> scanNetworks();

// WiFiManager.cpp
std::vector<std::string> WiFiManager::scanNetworks() {
    // Implementation
}
```

---

## Troubleshooting

### Library Not Found
- Verify `library.json` exists in library folder
- Check library folder structure matches expected layout
- Run `platformio lib list` to verify discovery

### Include Path Errors
- Ensure `freertos/FreeRTOS.h` is first include (if using FreeRTOS)
- Verify header paths match actual filenames
- Clean build: `platformio run -t clean`

### Compilation Errors
- Check for missing dependencies in `library.json`
- Verify framework/platform compatibility
- Review error messages for missing includes

---

## Version History

- **v1.0** (Feb 19, 2026) - Initial library structure with GPIO, HttpClient, WiFiManager, and MQTT
