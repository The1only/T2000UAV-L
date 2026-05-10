idf_component_register(
    SRCS
        "main.cpp"
        "wifi_manager.cpp"
        "telnet_server.cpp"
        "ssdp_responder.cpp"
        "serial_console.cpp"
        "wit_simulator.cpp"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash esp_wifi esp_event lwip driver esp_timer vfs
)
