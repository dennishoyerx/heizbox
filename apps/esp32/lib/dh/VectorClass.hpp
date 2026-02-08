#pragma once

namespace dh {

template <typename OBJ, typename KEY>
class Vector {
protected:
    std::vector<OBJ> objects;

    virtual const KEY& keyOf(const OBJ& obj) const = 0;

public:
    bool exists(const KEY& id) const {
        return std::any_of(objects.begin(), objects.end(),
            [&](const OBJ& o) { return keyOf(o) == id; });
    }

    OBJ* get(const KEY& id) {
        auto it = std::find_if(objects.begin(), objects.end(),
            [&](const OBJ& o) { return keyOf(o) == id; });

        return it != objects.end() ? &*it : nullptr;
    }

    void add(const OBJ& obj) {
        objects.push_back(obj);
    }

    bool remove(const KEY& id) {
        auto it = std::find_if(objects.begin(), objects.end(),
            [&](const OBJ& o) { return keyOf(o) == id; });

        if (it == objects.end()) return false;
        objects.erase(it);
        return true;
    }
};

};