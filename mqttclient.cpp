#ifdef V5

#include "mqttclient.h"
#include <iostream>

MqttClient::MqttClient(const std::string& server, const std::string& clientId)
    : client_(server, clientId),
    cb_(this) // this is key!
{
    client_.set_callback(cb_);
}

void MqttClient::connect() {
    try {
        client_.connect()->wait();
        std::cout << "Connected to broker.\n";
    } catch (const mqtt::exception& e) {
        std::cerr << "Connect failed: " << e.what() << '\n';
    }
}

void MqttClient::disconnect() {
    try {
        client_.disconnect()->wait();
        std::cout << "Disconnected from broker.\n";
    } catch (const mqtt::exception& e) {
        std::cerr << "Disconnect failed: " << e.what() << '\n';
    }
}

void MqttClient::sendMessage(const std::string& topic, const std::string& message) {
    try {
        client_.publish(topic, message.c_str(), message.length(), 1, false);
        std::cout << "eMove App Sent: " << message << '\n';
    } catch (const mqtt::exception& e) {
        std::cerr << "Publish failed: " << e.what() << '\n';
    }
}

void MqttClient::subscribe(const std::string& topic) {
    try {
        client_.subscribe(topic, 1)->wait();
        std::cout << "Subscribed to client topic: " << topic << '\n';
    } catch (const mqtt::exception& e) {
        std::cerr << "Subscribe failed: " << e.what() << '\n';
    }
}

void MqttClient::setMessageHandler(std::function<void(const std::string&, const std::string&)> handler) {
    userMessageHandler_ = handler;
}

void MqttClient::handleIncoming(const std::string& topic, const std::string& payload) {
    if (userMessageHandler_) {
        userMessageHandler_(topic, payload);  // Pass to main.cpp logic
    }
}

// --- Callback Implementation ---
void MqttClient::Callback::message_arrived(mqtt::const_message_ptr msg) {
    owner_->handleIncoming(msg->get_topic(), msg->to_string());
}

void MqttClient::Callback::connection_lost(const std::string& cause) {
    std::cerr << "MQTT client connection lost: " << cause << std::endl;
}

#else

#include "mqttClient.h"
#include <stdexcept>

MqttClient::MqttClient(const std::string& server, const std::string& clientId)
    : client_{server, clientId}, cb_{this}
{
    client_.set_callback(cb_);
}

void MqttClient::connect(bool cleanSession, std::chrono::seconds keepAlive)
{
    auto connOpts = mqtt::connect_options_builder()
    .clean_session(cleanSession)          // v3
        .keep_alive_interval(keepAlive)       // v3
        .finalize();

    // No connect_options_v5, no properties
    try {
        auto tok = client_.connect(connOpts);
        tok->wait(); // synchronous wait, keep it simple
    }catch(const mqtt::exception&) {
        // ignore on shutdown
    }
}

void MqttClient::disconnect()
{
    try {
        auto tok = client_.disconnect();
        tok->wait();
    } catch (...) {
        // ignore errors on disconnect for simplicity
    }
}


void MqttClient::sendMessage(const std::string& topic,
                             const std::string& payload,
                             int qos,
                             bool retained)
{
    auto msg = mqtt::make_message(topic, payload); // v3 message
    msg->set_qos(qos);
    msg->set_retained(retained);
    client_.publish(msg)->wait();
}

void MqttClient::subscribe(const std::string& topic, int qos)
{
    try {
        client_.subscribe(topic, qos)->wait();
    } catch(const mqtt::exception&) {
        // ignore on shutdown
    }
}

void MqttClient::Callback::message_arrived(mqtt::const_message_ptr msg)
{
    if (owner_->userMessageHandler_)
    {
        owner_->userMessageHandler_(msg->get_topic(), msg->to_string());
    }
}

void MqttClient::Callback::connection_lost(const std::string& cause)
{
    // You can add retry logic here if desired (still v3-only)
    (void)cause;
}

#endif
