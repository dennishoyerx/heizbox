```mermaid
classDiagram
    direction TD

    class Device {
        +setup()
        +loop()
        -HeaterController _heaterController
        -InputManager _inputManager
        -ScreenManager _screenManager
        -DisplayDriver _displayDriver
        -DeviceState _deviceState
        -WiFiManager _wifiManager
        -EventBus _eventBus
    }

    class EventBus {
        <<Core>>
        +publish(event)
        +subscribe(handler)
    }

    class DeviceState {
        <<State & StatsManager>>
        +uint16_t heatingDuration
        +uint16_t targetTemp
        +uint32_t totalHeatCycles
        +loadState()
        +saveState()
    }

    class Preferences {
        <<Library>>
        -begin()
        -put...()
        -get...()
    }

    class HeaterController {
        <<Hardware>>
        +startHeating(duration)
        +stopHeating()
        +setPower(level)
        -HeaterLogic _logic
    }
    note for HeaterController "Steuert MOSFET (Heater-Pin)
Liest Temperatur (ADC-Pin)"

    class HeaterLogic {
        <<Logic>>
        +calculatePID()
        +checkSafetyLimits()
    }

    class InputManager {
        <<Hardware>>
        +readInputs()
        +getLastEvent()
    }
    note for InputManager "Liest 5-Wege-Joystick (GPIO-Pins)"

    class DisplayDriver {
        <<Hardware>>
        +init()
        +drawBitmap()
        +printText()
        -TFT_eSPI_Driver _tft
    }
    note for DisplayDriver "Wrapper für TFT_eSPI"

    class TFT_eSPI {
        <<Library>>
    }

    class ScreenManager {
        <<UI>>
        +showScreen(screenId)
        +update()
        -Screen* _currentScreen
        -StatusBar _statusBar
    }

    class Screen {
        <<Abstract UI>>
        +render()
        +onEnter()
        +onExit()
        +handleInput(event)
    }

    class StartupScreen
    class MainMenuScreen
    class FireScreen
    class OtaUpdateScreen

    class StatusBar {
        <<UI Component>>
        +render(wifiState, batteryLevel)
    }

    class WiFiManager {
        <<Network>>
        +connect()
        +disconnect()
        -WebSocketManager _wsManager
    }
    note for WiFiManager "REST/WebSocket für Backend-Kommunikation"

    class WebSocketManager {
        <<Library>>
        +connect(url)
        +send(data)
    }

    ' --- Beziehungen ---
    Device *-- EventBus
    Device *-- HeaterController
    Device *-- InputManager
    Device *-- ScreenManager
    Device *-- DisplayDriver
    Device *-- DeviceState
    Device *-- WiFiManager

    ' Manager kommunizieren über den EventBus, nicht direkt
    InputManager ..> EventBus : publishes InputEvent
    HeaterController ..> EventBus : publishes HeaterEvent
    ScreenManager ..> EventBus : subscribes to events
    WiFiManager ..> EventBus : publishes NetworkEvent

    ' State-Management
    DeviceState ..> Preferences : uses for NVS

    ' UI-Struktur
    ScreenManager o-- Screen : manages active
    ScreenManager o-- StatusBar : owns/updates
    StartupScreen --|> Screen
    MainMenuScreen --|> Screen
    FireScreen --|> Screen
    OtaUpdateScreen --|> Screen
    FireScreen ..> HeaterController : (via EventBus)
    Screen ..> DisplayDriver : uses for rendering

    ' Hardware/Library-Abstraktion
    DisplayDriver ..> TFT_eSPI : uses
    HeaterController *-- HeaterLogic : uses

    ' Netzwerk
    WiFiManager *-- WebSocketManager
```