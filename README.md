# ESP32 IoT Project

A comprehensive IoT application built for ESP32 microcontrollers with support for WiFi connectivity, HTTP requests, MQTT messaging, GPIO control, and Telegram integration.

## Features

- **WiFi Connectivity**: Easy WiFi connection management with configurable credentials
- **HTTP Client**: Support for HTTP GET and POST requests to external APIs
- **MQTT Support**: MQTT connection for IoT messaging and pub/sub functionality
- **GPIO Control**: Digital input/output control for sensors and actuators
- **Telegram Integration**: Send messages directly to Telegram via HTTP API
- **FreeRTOS Architecture**: Multi-threaded task support for concurrent operations

## Hardware Requirements

- **Microcontroller**: ESP32 (ESP-WROVER-KIT or compatible)
- **Memory**: PSRAM support enabled for larger buffer operations
- **Connection**: WiFi enabled board

## Project Structure

```
├── src/                          # Main application source code
│   ├── main.cpp                 # Entry point (app_main function)
│   ├── config.hpp               # Global configuration and FreeRTOS includes
│   ├── CMakeLists.txt           # CMake build configuration for src
│   └── Examples/                # Example implementations
│       ├── examples.hpp         # Examples header declarations
│       ├── GPIO_example.cpp     # GPIO usage examples
│       └── Wifi_Connection_example.cpp # WiFi and MQTT examples
│
├── lib/                          # Local reusable libraries (PlatformIO managed)
│   ├── GPIO/                    # GPIO abstraction library
│   │   ├── GPIO.hpp             # GPIO class declaration
│   │   ├── GPIO.cpp             # GPIO implementation (digital & analog)
│   │   └── library.json         # PlatformIO library metadata
│   │
│   ├── HttpClient/              # HTTP client library
│   │   ├── HttpClient.hpp       # HTTP client class declaration
│   │   ├── HttpClient.cpp       # HTTP GET/POST/Telegram implementation
│   │   └── library.json         # PlatformIO library metadata
│   │
│   ├── WiFiManager/             # WiFi connection management library
│   │   ├── WiFiManager.hpp      # WiFiManager class declaration
│   │   ├── WiFiManager.cpp      # WiFi event handling implementation
│   │   └── library.json         # PlatformIO library metadata
│   │
│   ├── Mqtt_Connection/         # MQTT client library
│   │   ├── Mqtt_Connection.hpp  # MQTT class declaration
│   │   ├── Mqtt_Connection.cpp  # MQTT pub/sub implementation
│   │   └── library.json         # PlatformIO library metadata
│   │
│   └── README.md                # Libraries documentation
│
├── components/                   # ESP-IDF components (if needed)
├── include/                      # Additional header files
├── test/                         # Unit tests
├── CMakeLists.txt               # Root CMake build configuration
├── platformio.ini               # PlatformIO project configuration
├── sdkconfig.esp-wrover-kit    # ESP-IDF SDK configuration
└── README.md                     # This file
```

## Configuration

### Build Environment

The project uses **PlatformIO** for build management and **ESP-IDF** as the framework.

**platformio.ini settings:**
- Platform: Espressif32
- Board: ESP-WROVER-KIT
- Framework: ESP-IDF
- Monitor Speed: 115200 baud
- PSRAM: Enabled

### WiFi Configuration

Modify credentials in `src/main.cpp`:

```cpp
WiFiManager wifi("YOUR_SSID", "YOUR_PASSWORD");
```

### Telegram Integration

Configure Telegram bot token and chat ID:

```cpp
std::string token = "YOUR_BOT_TOKEN";
std::string chat_id = "YOUR_CHAT_ID";
```

## Building and Flashing

### Using PlatformIO

```bash
# Build the project
platformio run

# Upload to ESP32
platformio run --target upload

# Monitor serial output
platformio run --target monitor
```

### Using CMake

```bash
mkdir build
cd build
cmake ..
make
```

## Usage Examples

### WiFi Connection

```cpp
WiFiManager wifi("SSID", "Password");
if (wifi.connect() == ESP_OK) {
    // Connected successfully
}
```

### HTTP GET Request

```cpp
HttpClient http;
std::string response = http.get("https://jsonplaceholder.typicode.com/posts/1");
```

### HTTP POST Request

```cpp
HttpClient http;
std::string result = http.post("https://jsonplaceholder.typicode.com/posts", 
                               "{\"status\":\"ok\"}");
```

### Send Telegram Message

```cpp
HttpClient http;
http.sendTelegramMessage("BOT_TOKEN", "CHAT_ID", "Hello from ESP32!");
```

### GPIO Operations

See `src/Examples/GPIO_example.cpp` for GPIO control examples.

### MQTT Communication

See `src/Mqtt_Connection/` for MQTT client implementation and usage.

## Task Management

The project uses FreeRTOS for concurrent task execution:

```cpp
xTaskCreate(telegram_task, "telegram_task", 8192, NULL, 5, NULL);
```

## Dependencies

- ESP-IDF (embedded framework)
- FreeRTOS (real-time operating system)
- Standard C++ library

## Serial Monitor Configuration

- Baud Rate: 115200
- Core Debug Level: 3 (verbose logging)

## Troubleshooting

### PSRAM Issues
If experiencing PSRAM cache errors, ensure the following build flags are set:
```
-DBOARD_HAS_PSRAM
-mfix-esp32-psram-cache-issue
```

### WiFi Connection Failures
- Verify SSID and password are correct
- Ensure WiFi is enabled on your network
- Check that the WiFi is using 2.4 GHz (ESP32 limitation)

### HTTP Requests Failing
- Verify internet connectivity through WiFi
- Check that the target endpoint is accessible
- Review SSL/TLS certificate validation settings

## Future Enhancements

- Cloud integration (AWS IoT, Azure IoT Hub)
- Data logging to SD card
- OTA (Over-The-Air) firmware updates
- Web dashboard interface
- Advanced power management

## License

MIT license 

## Author

[Add author information here]

## Revision History

- **v1.0** (Feb 19, 2026) - Initial project structure and core functionality
