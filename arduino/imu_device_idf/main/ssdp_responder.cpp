#include "main.hpp"
#include <cstring>
#include <string>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/ip4_addr.h"
#include "esp_netif.h"

static const char* TAG = "ssdp";

static std::string make_response(const char* local_ip) {
    // Minimal UPnP-like response; tune this to match your original Arduino payloads if needed.
    // Note: iOS devices often don't actively M-SEARCH; your original code "blinded" broadcasts.
    SensorMode mode = get_sensor_mode();
    const char* name = sensor_mode_name(mode);

    char buf[512];
    std::snprintf(buf, sizeof(buf),
        "HTTP/1.1 200 OK\r\n"
        "CACHE-CONTROL: max-age=120\r\n"
        "EXT:\r\n"
        "LOCATION: http://%s/desc.xml\r\n"
        "SERVER: esp-idf/5.x UPnP/1.1 %s/1.0\r\n"
        "ST: Airplane-device\r\n"
        "USN: uuid:imu-device-%s\r\n"
        "\r\n",
        local_ip ? local_ip : "0.0.0.0",
        name,
        name
    );
    return std::string(buf);
}

static bool get_local_ip(char* out, size_t out_sz) {
    esp_netif_ip_info_t ip;
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (!netif) return false;
    if (esp_netif_get_ip_info(netif, &ip) != ESP_OK) return false;
    std::snprintf(out, out_sz, IPSTR, IP2STR(&ip.ip));
    return true;
}

static void ssdp_task(void* arg) {
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "socket() failed");
        vTaskDelete(nullptr);
        return;
    }

    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(CONFIG_SSDP_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) != 0) {
        ESP_LOGE(TAG, "bind() failed");
        close(sock);
        vTaskDelete(nullptr);
        return;
    }

    ESP_LOGI(TAG, "SSDP/Discovery UDP listening on port %d", CONFIG_SSDP_PORT);

    char rx[1024];
    while (true) {
        sockaddr_in from{};
        socklen_t fromlen = sizeof(from);
        int len = recvfrom(sock, rx, sizeof(rx)-1, 0, (sockaddr*)&from, &fromlen);
        if (len <= 0) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }
        rx[len] = 0;

        // Very forgiving match
        if (strstr(rx, "M-SEARCH") || strstr(rx, "ssdp:discover") || strstr(rx, "DISCOVER")) {
            char ip[32] = {0};
            if (!get_local_ip(ip, sizeof(ip))) std::snprintf(ip, sizeof(ip), "0.0.0.0");
            std::string resp = make_response(ip);
            sendto(sock, resp.data(), resp.size(), 0, (sockaddr*)&from, fromlen);
        }
    }
}

void ssdp_start() {
    xTaskCreatePinnedToCore(ssdp_task, "ssdp", 4096, nullptr, 4, nullptr, 0);
}
