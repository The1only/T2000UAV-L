#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

// Wrapper around Paho MQTT C++ async_client.
// This header supports two modes:
//  - V5 defined:   use v5-capable build (but still via async_client).
//  - V5 not defined: "v3" style connect/publish/subscribe without v5 properties.

#include <mqtt/async_client.h>
#include <mqtt/iaction_listener.h>   // used in the non-V5 branch
#include <string>
#include <functional>
#include <chrono>
#include <atomic>

#ifdef V5

/**
 * @brief Thin wrapper for MQTT v5-style usage based on mqtt::async_client.
 *
 * This variant is used when V5 is defined. It exposes:
 *  - connect()/disconnect()
 *  - sendMessage(topic, payload)
 *  - subscribe(topic)
 *  - setMessageHandler() to receive incoming messages.
 *
 * Internally it uses a nested Callback class that forwards message_arrived()
 * and connection_lost() to the owning MqttClient instance.
 */
class MqttClient
{
public:
    /**
     * @brief Construct MQTT client wrapper.
     * @param server   Broker URI, e.g. "tcp://localhost:1883".
     * @param clientId Client identifier to use when connecting.
     */
    MqttClient(const std::string& server, const std::string& clientId);

    /**
     * @brief Connect to the broker (synchronous wrapper around async_client).
     *
     * Uses basic defaults (no custom properties, clean session).
     */
    void connect();

    /**
     * @brief Disconnect from the broker.
     */
    void disconnect();

    /**
     * @brief Publish a message with default QoS and retained=false.
     *
     * @param topic   Topic name.
     * @param message Payload as string.
     */
    void sendMessage(const std::string& topic, const std::string& message);

    /**
     * @brief Subscribe to a topic with default QoS (typically 1).
     */
    void subscribe(const std::string& topic);

    /**
     * @brief Internal hook: called by Callback::message_arrived().
     *
     * This simply forwards to the user-registered handler if any.
     */
    void handleIncoming(const std::string& topic, const std::string& payload);

    /**
     * @brief Register a user message handler for incoming messages.
     *
     * Signature: handler(topic, payload)
     */
    void setMessageHandler(std::function<void(const std::string&, const std::string&)> handler)
    {
        userMessageHandler_ = std::move(handler);
    }

    /**
     * @brief Direct access to underlying async_client (advanced use only).
     */
    mqtt::async_client& client() { return client_; }

private:
    /**
     * @brief Paho callback implementation: forwards events to MqttClient.
     */
    class Callback : public virtual mqtt::callback
    {
    public:
        explicit Callback(MqttClient* owner) : owner_(owner) {}

        void message_arrived(mqtt::const_message_ptr msg) override;
        void connection_lost(const std::string& cause) override;

    private:
        MqttClient* owner_;  ///< Back-reference to owning MqttClient.
    };

    /// User-provided handler for incoming messages (may be empty).
    std::function<void(const std::string&, const std::string&)> userMessageHandler_;

    /// Underlying Paho asynchronous client.
    mqtt::async_client client_;

    /// Callback instance registered with async_client.
    Callback cb_;
};

#else   // ----------------------- non-V5 branch ------------------------------

class MqttClient;

/**
 * @brief Simple connection listener for connect() operations.
 *
 * This uses the legacy mqtt::iaction_listener interface to inform
 * the owning MqttClient about connect success/failure.
 */
class ConnListener : public virtual mqtt::iaction_listener
{
public:
    explicit ConnListener(MqttClient* o) : owner(o) {}

    void on_failure(const mqtt::token& tok) override;
    void on_success(const mqtt::token& tok) override;

private:
    MqttClient* owner = nullptr;
};

/**
 * @brief Wrapper around mqtt::async_client for "v3-style" MQTT usage.
 *
 * This version avoids v5-only types (no properties or reason codes)
 * and exposes:
 *  - connect(cleanSession, keepAlive)
 *  - disconnect()
 *  - sendMessage(topic, payload, qos, retained)
 *  - subscribe(topic, qos)
 *
 * Incoming messages are forwarded to a user-provided MsgHandler.
 */
class MqttClient
{
public:
    /// User message handler type: handler(topic, payload).
    using MsgHandler = std::function<void(const std::string&, const std::string&)>;

    /**
     * @brief Construct MQTT client wrapper.
     * @param server   Broker URI, e.g. "tcp://localhost:1883".
     * @param clientId Client identifier to use when connecting.
     */
    MqttClient(const std::string& server, const std::string& clientId);

    /**
     * @brief Connect to the broker (v3 style).
     *
     * @param cleanSession Whether to request a clean session.
     * @param keepAlive    Keep-alive interval.
     */
    void connect(bool cleanSession = true,
                 std::chrono::seconds keepAlive = std::chrono::seconds{20});

    /**
     * @brief Disconnect from the broker.
     */
    void disconnect();

    /**
     * @brief Publish a message.
     *
     * @param topic    Topic name.
     * @param payload  Message payload.
     * @param qos      QoS level (0, 1, or 2).
     * @param retained Retained flag.
     */
    void sendMessage(const std::string& topic,
                     const std::string& payload,
                     int qos = 1,
                     bool retained = false);

    /**
     * @brief Subscribe to a topic.
     *
     * @param topic Topic filter to subscribe.
     * @param qos   Desired QoS for subscription.
     */
    void subscribe(const std::string& topic, int qos = 1);

    /**
     * @brief Register user message handler.
     *
     * Called whenever a new message arrives on subscribed topics.
     */
    void setMessageHandler(MsgHandler handler) { userMessageHandler_ = std::move(handler); }

    /**
     * @brief Direct access to underlying async_client (advanced use only).
     */
    mqtt::async_client& client() { return client_; }

private:
    /**
     * @brief Paho callback implementation: forwards events to MqttClient.
     */
    class Callback : public virtual mqtt::callback
    {
    public:
        explicit Callback(MqttClient* owner) : owner_(owner) {}

        void message_arrived(mqtt::const_message_ptr msg) override;
        void connection_lost(const std::string& cause) override;

    private:
        MqttClient* owner_ = nullptr;
    };

    /// User-provided handler for incoming messages.
    MsgHandler userMessageHandler_;

    /// Underlying Paho asynchronous client.
    mqtt::async_client client_;

    /// Callback instance registered with async_client.
    Callback cb_;
};

#endif  // V5 / !V5

#endif // MQTTCLIENT_H
