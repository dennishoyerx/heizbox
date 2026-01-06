#pragma once

// Forward Declarations für alle wichtigen Klassen
// Nutze diese in Headers statt #include um Compile-Zeit zu reduzieren

// Core
class Device;
class DeviceState;
class EventBus;

// Heater
class HeaterController;
class HeaterMonitor;
class HeaterCycle;
class Sensors;
class ZVSDriver;
class TempSensor;
class IRTempSensor;
class ITemperatureSensor;

// UI
class DeviceUI;
class Screen;
class ScreenManager;
class DisplayDriver;
class StatusBar;
class UI;

// UI Screens
class FireScreen;
class StartupScreen;
class OtaUpdateScreen;
class TimezoneScreen;
class GenericMenuScreen;

// Input
class InputManager;
class InputHandler;

// Network
class Network;
class WiFiManager;
class WebSocketManager;
class OTASetup;

// Menu
class MenuItem;
class MenuBuilder;

// Hardware
class CapacitiveSensor;

// Structs (oft in Headers vollständig benötigt)
struct InputEvent;
struct HardwareConfig;
struct DisplayConfig;
struct HeaterConfig;