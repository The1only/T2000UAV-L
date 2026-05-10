#include "main.hpp"
#include <vector>
#include <algorithm>
#include <cstring>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"


// START_DIR=$(pwd)
// cd ~/esp/esp-idf-v5.1 && source ./export.sh
// cd "$START_DIR"
// idf.py set-target esp32c6
// idf.py build
// idf.py flash monitor

static const char* TAG = "telnet";

static SemaphoreHandle_t s_clients_mutex;
static std::vector<int> s_clients;

static void close_client(int fd) {
    if (fd >= 0) {
        shutdown(fd, SHUT_RDWR);
        close(fd);
    }
}

static void telnet_accept_task(void* arg) {
    const int port = CONFIG_TELNET_PORT;

    int listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_fd < 0) {
        ESP_LOGE(TAG, "socket() failed");
        vTaskDelete(nullptr);
        return;
    }

    int yes = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) != 0) {
        ESP_LOGE(TAG, "bind() failed");
        close(listen_fd);
        vTaskDelete(nullptr);
        return;
    }
    if (listen(listen_fd, 4) != 0) {
        ESP_LOGE(TAG, "listen() failed");
        close(listen_fd);
        vTaskDelete(nullptr);
        return;
    }

    ESP_LOGI(TAG, "Telnet server listening on port %d", port);

    while (true) {
        sockaddr_in6 source_addr{};
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_fd, (sockaddr*)&source_addr, &addr_len);
        if (sock < 0) {
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        // Non-blocking send is not strictly required; keep it simple.
        xSemaphoreTake(s_clients_mutex, portMAX_DELAY);
        s_clients.push_back(sock);
        xSemaphoreGive(s_clients_mutex);

        ESP_LOGI(TAG, "Client connected (fd=%d)", sock);
        const char* banner = "imu_device_idf: connected\r\n";
        send(sock, banner, strlen(banner), 0);
    }
}

void telnet_start() {
    s_clients_mutex = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(telnet_accept_task, "telnet_accept", 4096, nullptr, 5, nullptr, 0);
}

void telnet_broadcast(const uint8_t* data, size_t len) {
    if (!data || len == 0) return;

    xSemaphoreTake(s_clients_mutex, portMAX_DELAY);
    // remove dead clients as we go
    for (auto it = s_clients.begin(); it != s_clients.end(); ) {
        int fd = *it;
        int r = send(fd, data, len, 0);
        if (r < 0) {
            ESP_LOGW(TAG, "Dropping client fd=%d (send failed)", fd);
            close_client(fd);
            it = s_clients.erase(it);
        } else {
            ++it;
        }
    }
    xSemaphoreGive(s_clients_mutex);
}
