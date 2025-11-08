#include "ui/screens/FireScreen.h"
#include "core/DeviceState.h"
#include "ui/base/ScreenManager.h"
#include "ui/components/UIText.h"
#include "bitmaps.h"
#include <TFT_eSPI.h>
#include "utils/Logger.h"
#include "StateManager.h"
#include <utility>

FireScreen::FireScreen(HeaterController &hc, ScreenManager *sm,
                       ScreensaverScreen *ss, StatsManager *stm,
                       std::function<void(int)> setCycleCb)
    : heater(hc),
      screenManager(sm),
      screensaverScreen(ss),
      statsManager(stm),
      setCycleCallback(std::move(setCycleCb)),
      cachedClicks(0),
      cachedConsumption(0),
      cachedTodayConsumption(0),
      cachedYesterdayConsumption(0),
      cachedCaps(0)
{
    // State-Listener registrieren
    DeviceState::instance().sessionClicks.addListener([this](int clicks)
                                                      {
        cachedClicks = clicks;
        markDirty(); });

    DeviceState::instance().sessionCaps.addListener([this](int caps)
                                                    {
        cachedCaps = caps;
        markDirty(); });

    DeviceState::instance().sessionConsumption.addListener([this](float consumption)
                                                           {
        cachedConsumption = consumption;
        markDirty(); });

    DeviceState::instance().todayConsumption.addListener([this](float consumption)
                                                         {
        this->cachedTodayConsumption = consumption;
        markDirty(); });

    DeviceState::instance().yesterdayConsumption.addListener([this](float consumption)
                                                             {
        this->cachedYesterdayConsumption = consumption;
        markDirty(); });

    state.lastActivityTime = millis();
    state.currentCycle = 1;
    state.showingSavedConfirmation = false;
    state.confirmationStartTime = 0;
}

void FireScreen::onEnter()
{
    resetActivityTimer();
}

void FireScreen::onCycleFinalized() {
    if (heater.getLastCycleDuration() > 10000) {
        setCycleCallback(state.currentCycle);
        state.currentCycle = (state.currentCycle == 1) ? 2 : 1;
    }
}

void drawSessionRow(DisplayDriver &display, String label, float consumption, int y, bool highlight = false)
{
    // Position und Größe der Card
    int x = 10;      // X-Position (10px vom Rand)
    int width = 260; // Breite der Card
    int height = 50; // Höhe der Card
    int radius = 8;  // Eckenradius
    uint16_t bgColor;
    uint16_t textColor;
    uint16_t shadowColor;

    auto &renderer = display.getRenderer();

    if (highlight)
    {
        bgColor = TFT_WHITE;  // Hintergrund weiß
        textColor = 0xFB40;   // Orange (#FF6A00)
        shadowColor = 0x8410; // Dunkelgrau
        renderer.fillSmoothRoundRect(x, y, width, height, radius, bgColor, textColor);
    }
    else
    {
        bgColor = 0xFB40;        // Hintergrund orange
        textColor = TFT_WHITE;   // Text weiß
        shadowColor = TFT_WHITE; // Dunkelgrau
        //renderer.fillSmoothRoundRect(x, y, width, height, radius, bgColor, textColor);
        renderer.fillSmoothRoundRect(x, y, width, height, radius, shadowColor, shadowColor);
        renderer.fillSmoothRoundRect(x + 2, y + 2, width - 4, height - 4, radius, bgColor, bgColor);
    }

    // Icon zeichnen (Uhr-Symbol)
    int iconX = x + 12;
    int iconY = y + 8;
    int iconSize = 20;

    // Kreis für Uhr
    renderer.drawCircle(iconX + iconSize / 2, iconY + iconSize / 2, iconSize / 2, textColor);
    renderer.drawCircle(iconX + iconSize / 2, iconY + iconSize / 2, iconSize / 2 - 1, textColor);

    // Zeiger der Uhr
    int centerX = iconX + iconSize / 2;
    int centerY = iconY + iconSize / 2;
    renderer.drawLine(centerX, centerY, centerX, centerY - 6, textColor);     // Minutenzeiger
    renderer.drawLine(centerX, centerY, centerX + 4, centerY + 2, textColor); // Stundenzeiger
    renderer.fillCircle(centerX, centerY, 2, textColor);                      // Zentrum

    renderer.setTextSize(1);

    // "Session" Text
    renderer.setTextColor(textColor);
    renderer.setTextDatum(ML_DATUM);           // Middle Left
    renderer.setFreeFont(&FreeSans12pt7b);
    renderer.drawString(label, iconX + iconSize + 10, y + height / 2);

    // Verbrauchswert formatieren und anzeigen
    char consumptionStr[10];
    int integer = (int)consumption;
    int decimal = (int)(consumption * 100) % 100;
    if (integer > 0)
    {
        sprintf(consumptionStr, "%d.%02dg", integer, decimal);
    }
    else
    {
        sprintf(consumptionStr, ".%02dg", decimal);
    }

    // Verbrauchswert rechts ausgerichtet
    renderer.setTextDatum(MR_DATUM);           // Middle Right
    renderer.setFreeFont(&FreeSansBold18pt7b);

    renderer.drawString(consumptionStr, x + width - 12, y + height / 2);
}


void drawSquare(DisplayDriver &display, int position, String label, float consumption, bool highlight = false)
{
    // Position und Größe der Card
    int width = 140;
    int height = 95;
    int x = 0; 
    int y = 0; 

    if (position == 1) {
        x = 140;
        y = 0;
    } else if (position == 2) {
        x = 0;
        y = 95;
    } else if (position == 3) {
        x = 140;
        y = 95;
    }

    int radius = 8;
    
    uint16_t bgColor;
    uint16_t textColor;
    uint16_t shadowColor;

    auto &renderer = display.getRenderer();

    if (highlight)
    {
        bgColor = TFT_WHITE;  // Hintergrund weiß
        textColor = 0xFB40;   // Orange (#FF6A00)
        shadowColor = 0x8410; // Dunkelgrau
        renderer.fillSmoothRoundRect(x, y, width, height, radius, bgColor, textColor);
    }
    else
    {
        bgColor = 0xFB40;        // Hintergrund orange
        textColor = TFT_WHITE;   // Text weiß
        shadowColor = TFT_WHITE; // Dunkelgrau
        //renderer.fillSmoothRoundRect(x, y, width, height, radius, bgColor, textColor);
        renderer.fillSmoothRoundRect(x, y, width, height, radius, shadowColor, shadowColor);
        renderer.fillSmoothRoundRect(x + 2, y + 2, width - 4, height - 4, radius, bgColor, bgColor);
    }

    // Icon zeichnen (Uhr-Symbol)
    int iconX = x + 12;
    int iconY = y + 8;
    int iconSize = 20;

    // Kreis für Uhr
    renderer.drawCircle(iconX + iconSize / 2, iconY + iconSize / 2, iconSize / 2, textColor);
    renderer.drawCircle(iconX + iconSize / 2, iconY + iconSize / 2, iconSize / 2 - 1, textColor);

    // Zeiger der Uhr
    int centerX = iconX + iconSize / 2;
    int centerY = iconY + iconSize / 2;
    renderer.drawLine(centerX, centerY, centerX, centerY - 6, textColor);     // Minutenzeiger
    renderer.drawLine(centerX, centerY, centerX + 4, centerY + 2, textColor); // Stundenzeiger
    renderer.fillCircle(centerX, centerY, 2, textColor);                      // Zentrum

    renderer.setTextSize(1);

    // "Session" Text
    renderer.setTextColor(textColor);
    renderer.setTextDatum(ML_DATUM);           // Middle Left
    renderer.setFreeFont(&FreeSans12pt7b); // oder eine andere verfügbare Font
    renderer.drawString(label, iconX + iconSize + 10, y + height - 15);

    // Verbrauchswert formatieren und anzeigen
    char consumptionStr[10];
    int integer = (int)consumption;
    int decimal = (int)(consumption * 100) % 100;
    if (integer > 0)
    {
        sprintf(consumptionStr, "%d.%02dg", integer, decimal);
    }
    else
    {
        sprintf(consumptionStr, ".%02dg", decimal);
    }

    // Verbrauchswert rechts ausgerichtet
    renderer.setTextDatum(MR_DATUM);           // Middle Right
    renderer.setFreeFont(&FreeSansBold18pt7b); // Größere Font für den Wert

    renderer.drawString(consumptionStr, x + width - 12, y + 15);
}

void FireScreen::draw(DisplayDriver &display)
{
    display.clear(); // Dark gray background

    drawStatus(display);
    //drawCycleInfo(display);
    //drawSessionStats(display);
    //drawSquare(display, 0, "Session", cachedConsumption, (state.currentCycle == 1));
    //drawSquare(display, 1, "Heute", cachedTodayConsumption);
    //drawSquare(display, 2, "Gestern", cachedYesterdayConsumption);
    drawSessionRow(display, "Session", cachedConsumption, 10, (state.currentCycle == 1));
    drawSessionRow(display, "Heute", cachedTodayConsumption, 65);
    drawSessionRow(display, "Gestern", cachedYesterdayConsumption, 120);

    if (heater.isHeating()) {
        drawHeatingTimer(display);
    }
}

void FireScreen::drawHeatingTimer(DisplayDriver &display)
{
    auto &renderer = display.getRenderer();
    
    const uint32_t elapsed = heater.getElapsedTime();
    const uint32_t seconds = elapsed / 1000;
    
    // Nur neu zeichnen bei Sekundenwechsel
    static uint32_t lastSeconds = 999;
    if (seconds == lastSeconds && heater.isHeating()) { // Only skip redraw if actively heating and second hasn't changed
        return;
    }
    lastSeconds = seconds;
    
    // Farbe basierend auf Zeit
    uint16_t timerColor;
    if (seconds < 20) {
        timerColor = 0x07E0;      // Grün
    } else if (seconds < 35) {
        timerColor = 0xFFE0;      // Gelb  
    } else if (seconds < 50) {
        timerColor = 0xFD20;      // Orange
    } else {
        timerColor = 0xF800;      // Rot
    }
    
    int centerX = 140;
    int centerY = 96;
    
    // === Vereinfachter Progress Ring ===
    int radius = 70;

    renderer.fillCircle(centerX, centerY, radius, 0xFB40);
    // Hintergrund-Ring
    renderer.drawCircle(centerX, centerY, radius, 0x8410);
    renderer.drawCircle(centerX, centerY, radius-1, 0x8410);
    renderer.drawCircle(centerX, centerY, radius-2, 0x8410);

    float progress = (float)seconds / 60.0f;
    int endAngle = (int)(progress * 360);
    int startAngle = 180;              // 12 Uhr bei Rotation 1
    int stopAngle = startAngle + endAngle;

    renderer.drawArc(centerX, centerY,
                    radius + 3, radius - 3,
                    startAngle, stopAngle,
                    timerColor, 0xFB40, true);

    // === Große Zahl ===
    char timeStr[4];
    snprintf(timeStr, sizeof(timeStr), "%lu", seconds % 60);
    
    renderer.setTextColor(TFT_WHITE);
    renderer.setTextDatum(MC_DATUM);
    renderer.setTextSize(2);
    renderer.setFreeFont(&FreeSansBold18pt7b);
    renderer.drawString(timeStr, centerX, centerY, 1);
    
    
    // "HEIZT" or "PAUSE" Badge
    const char* badgeText = heater.isPaused() ? "PAUSE" : "HEIZT";
    uint16_t badgeColor = heater.isPaused() ? TFT_YELLOW : timerColor;
    renderer.fillRoundRect(centerX - 35, centerY + 50, 70, 20, 10, 0x8410);
    renderer.fillCircle(centerX - 20, centerY + 60, 3, badgeColor);
    renderer.setFreeFont(&FreeSans18pt7b);
    renderer.setTextSize(1);
    renderer.drawString(badgeText, centerX + 5, centerY + 60, 2);
    
    // Click Zone
    if (seconds >= 30 && seconds <= 50) {
    renderer.setFreeFont(&FreeSans18pt7b);
        renderer.setTextSize(1);
        renderer.setTextColor(0xFFE0);
        renderer.drawString("CLICK ZONE", centerX, centerY + 80, 2);
    }
}

void FireScreen::drawStatus(DisplayDriver &display)
{
    const char *status = nullptr;

    switch (heater.getState())
    {
    case HeaterController::State::HEATING:
        status = nullptr;
        break; // Timer zeigt Status
    case HeaterController::State::PAUSED:
        status = "PAUSED";
        break;
    case HeaterController::State::COOLDOWN:
        status = "COOLDOWN";
        break;
    case HeaterController::State::ERROR:
        status = "ERROR";
        break;
    default:
        status = nullptr;
        break;
    }

    if (status)
    {
        // centerText(display, 180, status, TFT_WHITE, 2);
    }
}

void FireScreen::drawCycleInfo(DisplayDriver &display)
{
    char text[20];
    snprintf(text, sizeof(text), "%d", state.currentCycle);
    display.drawBitmap(10, 76, image_fire_48, 48, 48, TFT_WHITE);
    display.drawText(60, 114, text, TFT_WHITE, 4);
}

void FireScreen::drawSessionStats(DisplayDriver &display)
{
    char lineCaps[20];
    snprintf(lineCaps, sizeof(lineCaps), "%d", cachedCaps);

    char lineConsumption[20];
    snprintf(lineConsumption, sizeof(lineConsumption), "%.2fg", cachedConsumption);
    if (lineConsumption[0] == '0' && lineConsumption[1] == '.')
        memmove(lineConsumption, lineConsumption + 1, strlen(lineConsumption));

    char lineTodayConsumption[20];
    snprintf(lineTodayConsumption, sizeof(lineTodayConsumption), "%.2fg", cachedTodayConsumption);
    if (lineTodayConsumption[0] == '0' && lineTodayConsumption[1] == '.')
        memmove(lineTodayConsumption, lineTodayConsumption + 1, strlen(lineTodayConsumption));

    display.drawBitmap(160, 134, (state.currentCycle == 1) ? image_cap_fill_48 : image_cap_48, 48, 48, TFT_WHITE);
    display.drawBitmap(10, 134, image_session_48, 48, 48, TFT_WHITE);

    display.drawText(213, 168, lineCaps, TFT_WHITE, 3);
    display.drawText(63, 168, lineConsumption, TFT_WHITE, 3);
    display.drawText(160, 108, lineTodayConsumption, TFT_WHITE, 3);
}

void FireScreen::update()
{
    const bool isActive = heater.isHeating() || heater.isPaused() || heater.getState() == HeaterController::State::COOLDOWN;

    // Update heating timer
    if (isActive)
    {
        state.lastActivityTime = millis();

        // Redraw timer jede Sekunde
        static uint32_t lastSecond = 0;
        const uint32_t currentSecond = heater.getElapsedTime() / 1000;
        if (currentSecond != lastSecond)
        {
            markDirty();
            lastSecond = currentSecond;
        }
    }

    // If the heater just became paused, force a redraw to show "PAUSE" text
    static bool wasPaused = false;
    if (heater.isPaused() && !wasPaused) {
        markDirty();
    }
    wasPaused = heater.isPaused();


    checkScreensaverTimeout();
}

void FireScreen::handleInput(InputEvent event)
{
    if (event.type != PRESS)
        return;

    resetActivityTimer();

    // Determine if heating should be triggered
    bool triggerHeating = false;
    if (event.button == FIRE)
    {
        triggerHeating = true; // FIRE button always triggers heating
    }
    else if (event.button == CENTER && DeviceState::instance().enableCenterButtonForHeating.get())
    {
        triggerHeating = true; // CENTER button triggers heating if enabled
    }

    if (triggerHeating)
    {
        // If we are in any state other than active heating, start. Otherwise, pause.
        _handleHeatingTrigger(!heater.isHeating());
        return;
    }

    // Handle other inputs (UP/DOWN for cycle change)
    switch (event.button)
    {
    case UP:
        handleCycleChange();
        break;

    case DOWN:
        handleCycleChange();
        break;

    default:
        break;
    }
}

void FireScreen::handleCycleChange()
{
    if (state.currentCycle == 1)
    {
        state.currentCycle = 2;
    }
    else
    {
        state.currentCycle = 1;
    }

    setCycleCallback(state.currentCycle);
    markDirty();
}

void FireScreen::checkScreensaverTimeout()
{
    const bool isActive = heater.isHeating() || heater.isPaused() || heater.getState() == HeaterController::State::COOLDOWN;

    if (!isActive && (millis() - state.lastActivityTime > Timing::SCREENSAVER_TIMEOUT_MS))
    {
        screenManager->setScreen(screensaverScreen, ScreenTransition::FADE);
    }
}

void FireScreen::_handleHeatingTrigger(bool shouldStartHeating)
{
    if (shouldStartHeating)
    {
        // This will either start a new cycle or resume from a pause.
        heater.startHeating();
    }
    else
    {
        // This will pause the current cycle if it's heating.
        if (heater.isHeating()) {
            heater.stopHeating(false);
        }
    }
    markDirty(); // Always mark dirty when heating state changes
}

void FireScreen::resetActivityTimer()
{
    state.lastActivityTime = millis();
}
