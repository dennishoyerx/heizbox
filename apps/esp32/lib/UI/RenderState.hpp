#pragma once

#include <vector>
#include <functional>
#include <unordered_map>
#include <string>
#include <map>
#include <variant>

using StateValue = std::variant<int, float, bool, std::string>;

struct RenderStateHash {
  std::unordered_map<std::string, StateValue> values;
  size_t hash = 0;

  // Calculate hash from all values
  size_t calculateHash() const {
    size_t h = 0;
    for (const auto& [key, value] : values) {
      h ^= std::hash<std::string>{}(key);
      std::visit([&h](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>) {
          h ^= std::hash<std::string>{}(arg);
        } else if constexpr (std::is_same_v<T, float>) {
          h ^= std::hash<int>{}(static_cast<int>(arg * 1000)); // float precision
        } else {
          h ^= std::hash<T>{}(arg);
        }
      }, value);
    }
    return h;
  }

  bool hasChanged(const std::unordered_map<std::string, StateValue>& newValues) {
    size_t newHash = 0;
    for (const auto& [key, value] : newValues) {
      newHash ^= std::hash<std::string>{}(key);
      std::visit([&newHash](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>) {
          newHash ^= std::hash<std::string>{}(arg);
        } else if constexpr (std::is_same_v<T, float>) {
          newHash ^= std::hash<int>{}(static_cast<int>(arg * 1000));
        } else {
          newHash ^= std::hash<T>{}(arg);
        }
      }, value);
    }

    if (newHash != hash) {
      hash = newHash;
      values = newValues;
      return true;
    }
    return false;
  }
};