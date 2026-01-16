#include "Audio.h"
#include "Config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

namespace Audio {

// ===== Config =====
static constexpr int CH = 0;
static constexpr int RES = 8;
static constexpr uint8_t QUEUE_LEN = 24;

// ===== Internal Types =====
enum class CmdType : uint8_t { Tone, Silence, Chirp };

struct Cmd {
    CmdType type;
    uint32_t f0;
    uint32_t f1;
    uint16_t duration;
    uint8_t volume;
    bool fade;
};

// ===== State =====
static QueueHandle_t queue;
static TaskHandle_t task;
static uint8_t masterVolume = 70;
static bool muted = false;

// ===== Helpers =====
static inline uint8_t pwm(uint8_t volPct) {
    if (muted) return 0;
    volPct = constrain(volPct, 0, 100);
    uint16_t v = (volPct * masterVolume) / 100;
    return constrain(v * 255 / 100, 0, 255);
}

static void enqueue(const Cmd& c) {
    xQueueSend(queue, &c, 0);
}

static void tone(uint32_t f, uint16_t ms, uint8_t v, bool fade = false) {
    enqueue({CmdType::Tone, f, 0, ms, v, fade});
}

static void silence(uint16_t ms) {
    enqueue({CmdType::Silence, 0, 0, ms, 0, false});
}

static void chirp(uint32_t f0, uint32_t f1, uint16_t ms, uint8_t v) {
    enqueue({CmdType::Chirp, f0, f1, ms, v, false});
}

// ===== Audio Task =====
static void audioTask(void*) {
    Cmd c{};
    for (;;) {
        if (!xQueueReceive(queue, &c, portMAX_DELAY)) continue;

        if (c.type == CmdType::Silence) {
            ledcWrite(CH, 0);
            ledcWriteTone(CH, 0);
            vTaskDelay(pdMS_TO_TICKS(c.duration));
            continue;
        }

        if (c.type == CmdType::Chirp) {
            constexpr uint8_t steps = 20;
            uint16_t stepMs = c.duration / steps;
            int32_t df = ((int32_t)c.f1 - (int32_t)c.f0) / steps;

            for (uint8_t i = 0; i < steps; i++) {
                ledcWriteTone(CH, c.f0 + df * i);
                ledcWrite(CH, pwm(c.volume));
                vTaskDelay(pdMS_TO_TICKS(stepMs));
            }
            ledcWrite(CH, 0);
            ledcWriteTone(CH, 0);
            continue;
        }

        // ===== Tone =====
        ledcWriteTone(CH, c.f0);
        ledcWrite(CH, pwm(c.volume));

        if (!c.fade) {
            vTaskDelay(pdMS_TO_TICKS(c.duration));
        } else {
            uint16_t steady = c.duration * 7 / 10;
            uint16_t fade = c.duration - steady;
            vTaskDelay(pdMS_TO_TICKS(steady));

            uint8_t start = pwm(c.volume);
            for (int v = start; v > 0; v -= max(1, start / 8)) {
                ledcWrite(CH, v);
                vTaskDelay(pdMS_TO_TICKS(fade / 8));
            }
        }

        ledcWrite(CH, 0);
        ledcWriteTone(CH, 0);
    }
}

// ===== Public API =====
void init() {
    ledcSetup(CH, 2000, RES);
    ledcAttachPin(HardwareConfig::SPEAKER_PIN, CH);
    ledcWrite(CH, 0);

    queue = xQueueCreate(QUEUE_LEN, sizeof(Cmd));
    configASSERT(queue);

    BaseType_t ok = xTaskCreatePinnedToCore(
        audioTask, "audio", 2048, nullptr, 1, &task, 1
    );
    configASSERT(ok == pdPASS);
}

void setVolume(uint8_t percent) {
    masterVolume = constrain(percent, 0, 100);
}

void mute(bool on) {
    muted = on;
}

// ===== Sounds =====
void beepMenu() {
    tone(A4, 35, 15);

    /*tone(1800, 40, 60);
    silence(20);
    tone(2200, 30, 40);*/
}

void beepHeatCycleSwitch() {
    
    chirp(300, 650, 180, 45);

    return;
    tone(A4, 45, 65);
    silence(25);
    tone(C5, 45, 70);
}

void beepHeatStart() {
    chirp(300, 900, 220, 75);
    tone(1200, 120, 65);

    return;
    tone(E4, 60, 80);              // Kick
    silence(20);
    chirp(G4, E5, 220, 90);  
}

void beepHeatFinish() {
    
    // Wasserblub (tiefer, leicht moduliert)
    chirp(180, 140, 90, 55);
    silence(20);
    chirp(180, 140, 90, 55);
    silence(30);

    // Inhale (langer, smoother Sweep)
    chirp(400, 1200, 280, 70);

    // Release / Ausatmen
    tone(600, 220, 45, true);

    return;
    // Blub
    tone(C4, 40, 55);
    silence(25);
    tone(C4, 40, 55);
    silence(35);

    // Zug / Release
    chirp(E4, C5, 220, 70);
    tone(A4, 180, 50, true);
}

void beepError() {
    chirp(E5, A4, 200, 80);
    tone(E4, 180, 60, true);  
}

void beepSuccess() {
    tone(C5, 60, 80);
    silence(30);
    tone(E5, 60, 80);
    silence(30);
    tone(G5, 120, 100, true);
}

void beepStartup() {
    
    chirp(C4, G4, 180, 60);   // sanfter Aufstieg
    silence(40);
    tone(C5, 120, 55, true); 
}

void beepClick() {
    tone(2500, 25, 50);
}

void beepWarning() {
    for (uint8_t i = 0; i < 3; i++) {
        tone(1000, 100, 100);
        silence(100);
    }
}

void beepBatteryLow() {
    tone(800, 200, 80);
    silence(150);
    tone(600, 300, 70);
    silence(150);
    tone(400, 400, 60, true);
}

// ===== Melodies =====
static void playMelody(const uint16_t* n,
                       const uint16_t* d,
                       uint8_t len,
                       uint8_t v) {
    for (uint8_t i = 0; i < len; i++) {
        if (n[i] == REST) silence(d[i]);
        else tone(n[i], d[i], v);
        silence(20);
    }
}

void playSuperMario() {
    static const uint16_t n[] = {E5,E5,REST,E5,REST,C5,E5,REST,G5};
    static const uint16_t d[] = {100,100,100,100,100,100,100,100,200};
    playMelody(n, d, 9, 90);
}

void playTetris() {
    static const uint16_t n[] = {E5,B4,C5,D5,C5,B4,A4,A4,C5,E5,D5,C5,B4};
    static const uint16_t d[] = {100,50,50,100,50,50,100,50,50,100,50,50,100};
    playMelody(n, d, 13, 80);
}

}
