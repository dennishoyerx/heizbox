#pragma once

class IDevice {
    
};

class IDriver {
    virtual void init() = 0;
    virtual void update() = 0;
};

class InputDriver : public IDriver {};
class PCF8574Driver : public InputDriver {};
class EspIODriver : public InputDriver {};


class IService {
    virtual void init() = 0;
    virtual void update() = 0;
};