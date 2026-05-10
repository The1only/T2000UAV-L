#include "main.hpp"
#include <cstring>
#include <string>
#include <vector>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/event_groups.h"
#include "lwip/ip4_addr.h"

static const char* TAG = "wifi";

static EventGroupHandle_t s_wifi_event_group;
static constexpr int WIFI_CONNECTED_BIT = BIT0;
static constexpr int WIFI_FAIL_BIT      = BIT1;

static int s_retry_num = 0;

static void save_creds_to_nvs(const std::string& ssid, const std::string& pass) {
    nvs_handle_t nvs;
    if (nvs_open("wifi", NVS_READWRITE, &nvs) != ESP_OK) return;
    nvs_set_str(nvs, "ssid", ssid.c_str());
    nvs_set_str(nvs, "pass", pass.c_str());
    nvs_commit(nvs);
    nvs_close(nvs);
}

static bool load_creds_from_nvs(std::string& ssid, std::string& pass) {
    nvs_handle_t nvs;
    if (nvs_open("wifi", NVS_READONLY, &nvs) != ESP_OK) return false;

    size_t ssid_len = 0, pass_len = 0;
    if (nvs_get_str(nvs, "ssid", nullptr, &ssid_len) != ESP_OK || ssid_len == 0) { nvs_close(nvs); return false; }
    if (nvs_get_str(nvs, "pass", nullptr, &pass_len) != ESP_OK) { nvs_close(nvs); return false; }

    ssid.resize(ssid_len - 1);
    pass.resize(pass_len ? pass_len - 1 : 0);

    nvs_get_str(nvs, "ssid", ssid.data(), &ssid_len);
    if (pass_len) nvs_get_str(nvs, "pass", pass.data(), &pass_len);
    nvs_close(nvs);
    return true;
}

static void clear_creds_nvs() {
    nvs_handle_t nvs;
    if (nvs_open("wifi", NVS_READWRITE, &nvs) != ESP_OK) return;
    nvs_erase_key(nvs, "ssid");
    nvs_erase_key(nvs, "pass");
    nvs_commit(nvs);
    nvs_close(nvs);
}

struct ApCred { const char* ssid; const char* pass; };

// TODO: Replace with your real AP list (or remove entirely and rely on NVS)
static const ApCred kFallbackAps[] = {
    {"Hvattum", "Jordvarme@2023@"},
    {"Altibox177449", "HheX9Xac"},
    {"Aeros2", "Terjenilsen1"},
    {"9Tek_printer", "didiinne"},
};

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 10) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGW(TAG, "Retrying Wi-Fi...");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static bool connect_with_creds(const char* ssid, const char* pass, TickType_t timeout_ticks) {
    wifi_config_t wifi_config{};
    std::snprintf((char*)wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), "%s", ssid ? ssid : "");
    std::snprintf((char*)wifi_config.sta.password, sizeof(wifi_config.sta.password), "%s", pass ? pass : "");
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg = {.capable = true, .required = false};

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
    s_retry_num = 0;
    ESP_LOGI(TAG, "Connecting to SSID: %s", ssid);
    
    esp_err_t err = esp_wifi_connect();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to start WiFi connect: %s", esp_err_to_name(err));
        return false;
    }

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                          WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                          pdTRUE, pdFALSE, timeout_ticks);

    return (bits & WIFI_CONNECTED_BIT);
}

void wifi_start() {
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler, nullptr, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler, nullptr, nullptr));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    std::string ssid, pass;
    if (load_creds_from_nvs(ssid, pass)) {
        if (connect_with_creds(ssid.c_str(), pass.c_str(), pdMS_TO_TICKS(15000))) return;
        ESP_LOGW(TAG, "Stored credentials failed; falling back to compiled list.");
    } else {
        ESP_LOGI(TAG, "No stored Wi-Fi credentials in NVS; using fallback list.");
    }

    for (const auto& ap : kFallbackAps) {
        if (std::strlen(ap.ssid) == 0) continue;  // Skip empty entries
        if (connect_with_creds(ap.ssid, ap.pass, pdMS_TO_TICKS(12000))) {
            // Save the AP we successfully used (optional)
            save_creds_to_nvs(ap.ssid, ap.pass ? ap.pass : "");
            return;
        }
    }

    ESP_LOGE(TAG, "Failed to connect to Wi-Fi using all options.");
}

// Exposed for the console command handler
extern "C" void wifi_creds_set(const char* ssid, const char* pass) { save_creds_to_nvs(ssid?ssid:"", pass?pass:""); }
extern "C" void wifi_creds_clear() { clear_creds_nvs(); }
extern "C" bool wifi_creds_get(char* ssid_out, size_t ssid_sz, char* pass_out, size_t pass_sz) {
    std::string ssid, pass;
    if (!load_creds_from_nvs(ssid, pass)) return false;
    std::snprintf(ssid_out, ssid_sz, "%s", ssid.c_str());
    std::snprintf(pass_out, pass_sz, "%s", pass.c_str());
    return true;
}
