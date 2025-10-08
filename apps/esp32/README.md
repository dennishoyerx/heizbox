# Dynavap Induction Heater Firmware

This is the firmware for a custom-built induction heater for Dynavap vaporizers. The project is built in C++ using the PlatformIO ecosystem and the Arduino framework, targeting the ESP32 microcontroller.

The firmware features a modular architecture, with distinct manager classes for handling core functionalities like display, input, and screen navigation. It presents a user-friendly interface on a color TFT display, navigated with a 5-way joystick. The device connects to WiFi for time synchronization and data logging to a Cloudflare service.

## Key Technologies:

*   **Hardware:** ESP32, ST7789 TFT Display, 5-way Joystick, ZVS Induction Heater Module.
*   **Software:** C++, PlatformIO, Arduino Framework.
*   **Libraries:** `TFT_eSPI`, `Adafruit GFX`.

## Features

*   **Multiple Screens:** A user-friendly interface with several screens:
    *   **Fire Screen:** Shows heating status and a timer.
    *   **Main Menu:** Allows configuration of device settings.
    *   **Stats Screen:** Displays usage statistics like total and session cycle counts.
    *   **Screensaver:** A simple clock display to prevent screen burn-in.
    *   **Hidden Mode:** Turns off the display for discreet operation.
    *   **Startup Screen:** A brief startup animation.
*   **Heater Control:**
    *   Precise control over the induction heating element.
    *   Configurable auto-stop timer to prevent overheating.
    *   Cooldown period after heating.
*   **Usage Statistics:**
    *   Tracks total heating cycles.
    *   Tracks cycles within the current session.
    *   Calculates the average heating duration.
*   **Device Settings:**
    *   Adjustable screen brightness.
    *   Configuration for auto-stop time and sleep timeout.
    *   Settings are saved persistently in non-volatile memory.
*   **Connectivity:**
    *   Connects to WiFi to synchronize the internal clock with an NTP server.
    *   Displays WiFi status and time on a dedicated status bar.

## Hardware

*   **Microcontroller:** ESP32 Dev Board
*   **Display:** 240x280 ST7789 TFT Display
*   **Input:** 5-way Joystick (Up, Down, Left, Right, Center) and a separate Fire Button.
*   **Heater:** 120W ZVS Induction Heater Module
*   **Power:** 12V 20A Power Supply & 5V DC-DC Buck Converter
*   **Control:** MOSFET with an Optocoupler to safely trigger the heater module from the ESP32.

### Pinout

The pin connections are defined in the `platformio.ini` file:

| Function         | Pin |
| ---------------- | --- |
| **Display**      |     |
| TFT_MOSI         | 23  |
| TFT_SCLK         | 18  |
| TFT_CS           | 5   |
| TFT_DC           | 21  |
| TFT_RST          | 19  |
| TFT_BL (Backlight) | 22  |
| **Input**          |     |
| Fire Button      | 25  |
| Joystick Up      | 12  |
| Joystick Down    | 33  |
| Joystick Left    | 27  |
| Joystick Right   | 14  |
| Joystick Press   | 26  |
| **Heater**         |     |
| MOSFET Gate      | 13  |

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