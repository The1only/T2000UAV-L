#include "main.hpp"
#include <cstring>
#include <string>
#include <vector>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"

static const char* TAG = "console";

extern "C" void wifi_creds_set(const char* ssid, const char* pass);
extern "C" void wifi_creds_clear();
extern "C" bool wifi_creds_get(char* ssid_out, size_t ssid_sz, char* pass_out, size_t pass_sz);

static std::vector<std::string> split(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            if (!cur.empty()) { out.push_back(cur); cur.clear(); }
        } else cur.push_back(c);
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

static void print_help() {
    printf("\nCommands:\n");
    printf("  help\n");
    printf("  wifi show\n");
    printf("  wifi set <ssid> <pass>\n");
    printf("  wifi clear\n\n");
}

static void handle_line(const std::string& line) {
    auto toks = split(line);
    if (toks.empty()) return;

    if (toks[0] == "help" || toks[0] == "?") {
        print_help();
        return;
    }

    if (toks[0] == "wifi") {
        if (toks.size() >= 2 && toks[1] == "show") {
            char ssid[64]{}, pass[64]{};
            if (wifi_creds_get(ssid, sizeof(ssid), pass, sizeof(pass))) {
                printf("Stored SSID: %s\n", ssid);
                printf("Stored PASS: %s\n", pass[0] ? "(set)" : "(empty)");
            } else {
                printf("No stored credentials.\n");
            }
            return;
        }
        if (toks.size() >= 4 && toks[1] == "set") {
            wifi_creds_set(toks[2].c_str(), toks[3].c_str());
            printf("Saved. Reboot to apply.\n");
            return;
        }
        if (toks.size() >= 2 && toks[1] == "clear") {
            wifi_creds_clear();
            printf("Cleared. Reboot to apply.\n");
            return;
        }
    }

    printf("Unknown command. Type `help`.\n");
}

static void console_task(void* arg) {
    // Configure UART0 for console I/O (works with ESP-IDF monitor + USB serial)
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 4096, 0, 0, nullptr, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
    esp_vfs_dev_uart_use_driver(UART_NUM_0);

    print_help();

    std::string line;
    while (true) {
        int c = getchar();
        if (c == EOF) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        if (c == '\r') continue;
        if (c == '\n') {
            handle_line(line);
            line.clear();
        } else {
            line.push_back((char)c);
        }
    }
}

void serial_console_start() {
    xTaskCreatePinnedToCore(console_task, "console", 4096, nullptr, 3, nullptr, 0);
}
