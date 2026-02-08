#pragma once
#include <string>
#include <vector>
#include <array>
#include <algorithm>

/*
struct Profile {
    struct Temps {
        const int* data;
        size_t size;
    };

    using TempLimits = std::array<int, 2>;

    template <size_t N>
    struct Config {
        const char* name;
        Temps temps;
    };

    int cycles;

    template <size_t N>
    constexpr Profile(Config<N> config) : cycles(0) {};


    void test() {};
    static void testStatic() {
    };
};

constexpr Profile::Config <2>profile1 = {
    .name = "Classic",
    .temps = Profile::Temps{200, 210}
};


constexpr Profile x = Profile(profile1);

constexpr Profile profile_default = Profile(Profile::Config<2>{
    .name = "Default",
    .temps = {200, 210}
});


constexpr Profile profile_bong_onehit = Profile(Profile::Config<1>{
    .name = "Bong Onehit",
    .temps = {220}
});

void xx() {
//    profile_default.
   //Profile pp = new Profile(profile_bong_onehit);
};



class ProfileManager {
    std::vector<std::string> profiles;
    std::string currentProfile;


public:
    ProfileManager();

    void switchProfile(std::string profileName);
    void setProfile(Profile::Config<2> profile) {
        if (profileExists(profile.name)) return;
    }

    bool profileExists(std::string profileName) {
        return std::find(profiles.begin(), profiles.end(), profileName) != profiles.end();
    }

};*/