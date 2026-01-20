#pragma once

class IDriver {
public:
    virtual void init() = 0;
    virtual void update() = 0;
};

class IService {
public:
    virtual void init() = 0;
    virtual void update() = 0;
};

class TFT;
class IDisplayDriver: public IDriver {
    virtual void setBrightness(int brightness) = 0;
    virtual void setOrientation(bool flipped) = 0;
private:
    TFT& tft;
};

class WSSManager;
class WifiManager;
class OTAUpdateManager;
class INetworkService: public IService {
private:
    WifiManager& wifi;
    WSSManager& wss;
    OTAUpdateManager& ota;
};

class IUIService: public IService {
private:
    IDisplayDriver& driver;
};

class IDevice {    
public:
    virtual void init() = 0;
    virtual void update() = 0;
private:
    IUIService& ui;
    INetworkService& network;
};

class IHeaterService: public IService {
private:
    IHeater& heater;
};

class IHeater {};

class IHeaterController {
public:
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void startHeating() = 0;
    virtual void stopHeating() = 0;
};