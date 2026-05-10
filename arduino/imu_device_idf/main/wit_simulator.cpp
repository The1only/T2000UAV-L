#include "main.hpp"
#include <cmath>
#include <cstdint>
#include <cstring>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

// This is a small ESP-IDF translation of the WIT packet sender idea in your Arduino sketch.
// If you want the *exact* field scaling/packet IDs, paste the expected host parser or point
// me to it and I’ll mirror it precisely.

static const char* TAG = "wit";

static inline uint32_t ms() {
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
}

static void sendPacket4(uint8_t pid, int16_t x, int16_t y, int16_t z, int16_t t) {
    uint8_t pkt[11];
    pkt[0] = 0x55;
    pkt[1] = pid;
    pkt[2] = (uint8_t)(x & 0xFF);
    pkt[3] = (uint8_t)((x >> 8) & 0xFF);
    pkt[4] = (uint8_t)(y & 0xFF);
    pkt[5] = (uint8_t)((y >> 8) & 0xFF);
    pkt[6] = (uint8_t)(z & 0xFF);
    pkt[7] = (uint8_t)((z >> 8) & 0xFF);
    pkt[8] = (uint8_t)(t & 0xFF);
    pkt[9] = (uint8_t)((t >> 8) & 0xFF);

    uint8_t sum = 0;
    for (int i = 0; i < 10; i++) sum += pkt[i];
    pkt[10] = sum;

    telnet_broadcast(pkt, sizeof(pkt));
}

static void simulator_task(void* arg) {
    ESP_LOGI(TAG, "WIT simulator started");

    float yaw = 0.0f;
    uint32_t last = ms();

    while (true) {
        uint32_t now = ms();
        float dt = (now - last) / 1000.0f;
        last = now;

        // Simple motion
        yaw += 30.0f * dt; // deg/sec
        if (yaw > 180.0f) yaw -= 360.0f;

        // Encode as "angles" packet (0x53 in your sketch)
        // WIT typically uses 0.01 deg units scaled to int16.
        int16_t roll_raw  = 0;
        int16_t pitch_raw = 0;
        int16_t yaw_raw   = (int16_t)lrintf(yaw * 100.0f);

        sendPacket4(0x53, roll_raw, pitch_raw, yaw_raw, 0);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

extern "C" void wit_simulator_start() {
    xTaskCreatePinnedToCore(simulator_task, "wit_sim", 4096, nullptr, 4, nullptr, 1);
}
