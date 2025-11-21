#ifdef V5

#include "mqttclient.h"
#include <iostream>

// -----------------------------------------------------------------------------
//  MqttClient (V5-style usage, but still via async_client)
// -----------------------------------------------------------------------------

MqttClient::MqttClient(const std::string& server, const std::string& clientId)
    : client_(server, clientId)
    , cb_(this)   // Important: callback holds back-reference to this
{
    // Register our callback with the underlying Paho client
    client_.set_callback(cb_);
}

void MqttClient::connect()
{
    try {
        // Simple synchronous connect (blocks until result)
        client_.connect()->wait();
        std::cout << "Connected to broker.\n";
    }
    catch (const mqtt::exception& e) {
        std::cerr << "Connect failed: " << e.what() << '\n';
    }
}

void MqttClient::disconnect()
{
    try {
        client_.disconnect()->wait();
        std::cout << "Disconnected from broker.\n";
    }
    catch (const mqtt::exception& e) {
        std::cerr << "Disconnect failed: " << e.what() << '\n';
    }
}

void MqttClient::sendMessage(const std::string& topic, const std::string& message)
{
    try {
        // QoS 1, not retained
        client_.publish(topic, message.c_str(), message.length(), 1, false);
        std::cout << "eMove App Sent: " << message << '\n';
    }
    catch (const mqtt::exception& e) {
        std::cerr << "Publish failed: " << e.what() << '\n';
    }
}

void MqttClient::subscribe(const std::string& topic)
{
    try {
        client_.subscribe(topic, 1)->wait();
        std::cout << "Subscribed to client topic: " << topic << '\n';
    }
    catch (const mqtt::exception& e) {
        std::cerr << "Subscribe failed: " << e.what() << '\n';
    }
}

void MqttClient::setMessageHandler(
    std::function<void(const std::string&, const std::string&)> handler)
{
    userMessageHandler_ = std::move(handler);
}

void MqttClient::handleIncoming(const std::string& topic,
                                const std::string& payload)
{
    // Forward incoming messages to user handler if registered
    if (userMessageHandler_) {
        userMessageHandler_(topic, payload);
    }
}

// -----------------------------------------------------------------------------
//  Callback (Paho callback implementation, V5 branch)
// -----------------------------------------------------------------------------

void MqttClient::Callback::message_arrived(mqtt::const_message_ptr msg)
{
    if (owner_) {
        owner_->handleIncoming(msg->get_topic(), msg->to_string());
    }
}

void MqttClient::Callback::connection_lost(const std::string& cause)
{
    std::cerr << "MQTT client connection lost: " << cause << std::endl;
    // Optional: you could add auto-reconnect logic here if desired.
}

#else   // ------------------------------- non-V5 branch ------------------------

#include "mqttclient.h"
#include <stdexcept>

// -----------------------------------------------------------------------------
//  MqttClient (v3-style usage, no v5 properties)
// -----------------------------------------------------------------------------

MqttClient::MqttClient(const std::string& server, const std::string& clientId)
    : client_{server, clientId}
    , cb_{this}
{
    // Register our callback with the underlying Paho client
    client_.set_callback(cb_);
}

void MqttClient::connect(bool cleanSession, std::chrono::seconds keepAlive)
{
    // Build v3 connect options (no v5 properties used)
    auto connOpts = mqtt::connect_options_builder()
                        .clean_session(cleanSession)
                        .keep_alive_interval(keepAlive)
                        .finalize();

    try {
        auto tok = client_.connect(connOpts);
        tok->wait();  // Synchronous wait: keep API simple
    }
    catch (const mqtt::exception&) {
        // For simplicity we ignore connect failures here (often happens on shutdown);
        // caller can add logging around connect() if needed.
    }
}

void MqttClient::disconnect()
{
    try {
        auto tok = client_.disconnect();
        tok->wait();
    }
    catch (...) {
        // Ignore errors on disconnect; usually harmless during shutdown.
    }
}

void MqttClient::sendMessage(const std::string& topic,
                             const std::string& payload,
                             int qos,
                             bool retained)
{
    // v3-style message (no properties)
    auto msg = mqtt::make_message(topic, payload);
    msg->set_qos(qos);
    msg->set_retained(retained);

    // Block until publish completes (simple, but fine for your usage)
    client_.publish(msg)->wait();
}

void MqttClient::subscribe(const std::string& topic, int qos)
{
    try {
        client_.subscribe(topic, qos)->wait();
    }
    catch (const mqtt::exception&) {
        // Ignore on shutdown or transient errors; caller can add logging if needed.
    }
}

// -----------------------------------------------------------------------------
//  Callback (Paho callback implementation, non-V5 branch)
// -----------------------------------------------------------------------------

void MqttClient::Callback::message_arrived(mqtt::const_message_ptr msg)
{
    if (owner_ && owner_->userMessageHandler_) {
        owner_->userMessageHandler_(msg->get_topic(), msg->to_string());
    }
}

void MqttClient::Callback::connection_lost(const std::string& cause)
{
    // Currently we just drop the connection; auto-reconnect could be added here.
    (void)cause;
}

#endif  // V5
