#pragma once 

namespace dh {

class NVSObject {
public:
    NVSObject(const char* key, const char* name);

private:
    const char* key;
    const char* name;

};



}