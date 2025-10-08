# GEMINI Project Analysis: Dynavap Induction Heater

## Project Overview

This is the firmware for a custom-built induction heater for Dynavap vaporizers. The project is built in C++ using the PlatformIO ecosystem and the Arduino framework, targeting the ESP32 microcontroller.

The firmware features a modular architecture, with distinct manager classes for handling core functionalities like display, input, and screen navigation. It presents a user-friendly interface on a color TFT display, navigated with a 5-way joystick. The device connects to WiFi for time synchronization and data logging to a Cloudflare service.

**Key Technologies:**

*   **Hardware:** ESP32, ST7789 TFT Display, 5-way Joystick, ZVS Induction Heater Module.
*   **Software:** C++, PlatformIO, Arduino Framework.
*   **Libraries:** `TFT_eSPI`, `Adafruit GFX`.

## Building and Running

The project is configured to be built and managed using the PlatformIO IDE extension in Visual Studio Code.

**Prerequisites:**

1.  **Visual Studio Code** with the **PlatformIO IDE extension**.
2.  **WiFi Credentials:** A `credentials.h` file must be created in the `include/` directory with the following content:
    ```cpp
    #define WIFI_SSID "YourSSID"
    #define WIFI_PASSWORD "YourPassword"
    ```

**Build & Upload Commands:**

The standard PlatformIO commands are used for building, uploading, and monitoring:

*   **Build:** `pio run`
*   **Upload:** `pio run --target upload`
*   **Monitor (Serial):** `pio device monitor`
*   **Monitor (Telnet):** Connect a Telnet client to the device's IP address on port 23.

## Development Conventions

*   **Modular Architecture:** The code is organized into a `Device` class that manages several smaller, single-responsibility manager classes (e.g., `ScreenManager`, `InputManager`).
*   **Screen Management:** The UI is divided into distinct `Screen` classes, each responsible for a specific part of the user interface.
*   **Hardware Abstraction:** Pin definitions and hardware-specific configurations are managed through build flags in `platformio.ini`, keeping the core C++ code clean and portable.
*   **Logging:** The project uses a custom `TelnetLogger` to mirror serial output to a Telnet server, allowing for wireless debugging.
*   **Configuration:** Device settings are persisted in the ESP32's non-volatile storage (NVS).
