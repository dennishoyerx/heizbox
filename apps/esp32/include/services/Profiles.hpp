#pragma once
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <VectorClass.hpp>

struct Cycle {
    struct Config {
        /* data */
    };
    
};

struct Profile {
    using Id = const char*;

    struct CycleSetting {
        int targetTemp;
    };

    using CycleSettings = std::vector<CycleSetting>;

    struct Config {
        const Id name;
        CycleSettings temps;
    };

    Profile(Config config) : name(config.name), settings(config.temps) {};

private:
    Id name;
    CycleSettings settings;
};

const Profile profile_default = Profile(Profile::Config{
    .name = "Default",
    .temps = {{200}, {210}}
});


const Profile profile_bong_onehit = Profile(Profile::Config{
    .name = "Bong Onehit",
    .temps = {{220}}
});

class ProfileManager: public dh::Vector<Profile::Config, Profile::Id> {
protected:
    const Profile::Id& keyOf(const Profile::Config& p) const override {
        return p.name;
    }

public:
    void setProfile(const Profile::Config& profile) {
        if (exists(profile.name)) return;
        add(profile);
    }

    bool switchProfile(const Profile::Id& name) {
        return get(name) != nullptr;
    }
};

