#pragma once

#include "ui/ColorPalette.h"
#include <RenderSurface.h>
#include <TFT_eSPI.h>
#include <vector>
#include <functional>
#include <unordered_map>
#include <string>
#include <map>
#include <variant>


using SurfaceCallback = std::function<void(RenderSurface&)>;
using SurfaceCallbackWithState = std::function<void(RenderSurface&, std::unordered_map<std::string, StateValue>&)>;

class SurfaceFactory {
public:
  SurfaceFactory(TFT_eSPI* tft) : _tft(tft), _usePsram(false), _forceRedraw(false) {}
  ~SurfaceFactory();

  RenderSurface createSurface(int16_t w, int16_t h, bool clear = true);
  void releaseSurface(RenderSurface& s);

  // Original withSurface - always renders
  void withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY, SurfaceCallback cb, bool clear = true);

  // New withSurface with state tracking - only renders if state changed
  void withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY, 
                   const std::unordered_map<std::string, StateValue>& state,
                   SurfaceCallback cb, bool clear = true);

  void usePSRAM(bool en) { _usePsram = en; }
  
  // Force all surfaces to redraw on next render
  void forceRedraw() { _forceRedraw = true; }
  
  // Invalidate all cached states in the pool
  void invalidateAll() {
    for (auto& entry : _pool) {
      entry.stateHash.hash = 0;
      entry.stateHash.values.clear();
    }
  }

  void setDarkMode(bool dark);


private:
  TFT_eSPI* _tft;
  bool _usePsram;
  bool _forceRedraw;
  bool _darkMode;


  struct PoolEntry { 
    TFT_eSprite* sprite = nullptr; 
    int16_t w = 0;
    int16_t h = 0;
    RenderStateHash stateHash;
  };
  std::vector<PoolEntry> _pool;
};
