#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#ifdef V5
#include <mqtt/async_client.h>
#include <string>
#include <functional>

class MqttClient{

public:
    MqttClient(const std::string& server, const std::string& clientId);
    void connect();
    void disconnect();
    void sendMessage(const std::string& topic, const std::string& message);
    void subscribe(const std::string& topic);
    void handleIncoming(const std::string& topic, const std::string& payload);
    void setMessageHandler(std::function<void(const std::string&, const std::string&)> handler);

    mqtt::async_client& client() { return client_; }

private:
    class Callback : public virtual mqtt::callback {
    public:
        Callback(MqttClient* owner) : owner_(owner) {}
        void message_arrived(mqtt::const_message_ptr msg) override;
        void connection_lost(const std::string& cause) override;

    private:
        MqttClient* owner_;  // Back-reference to parent
    };
    std::function<void(const std::string&, const std::string&)> userMessageHandler_;

    mqtt::async_client client_;
    Callback cb_;
};

#else

#include <mqtt/async_client.h>   // v3/v3.1.1 APIs (we avoid v5-only types)
#include <string>
#include <functional>
#include <chrono>

#include <mqtt/iaction_listener.h>
#include <atomic>

class MqttClient;

class ConnListener : public virtual mqtt::iaction_listener {
public:
    explicit ConnListener(MqttClient* o) : owner(o) {}
    void on_failure(const mqtt::token& tok) override;
    void on_success(const mqtt::token& tok) override;
private:
    MqttClient* owner;
};



class MqttClient {
public:
    using MsgHandler = std::function<void(const std::string&, const std::string&)>;

    MqttClient(const std::string& server, const std::string& clientId);

    // v3 connect/disconnect (no v5 properties)
    void connect(bool cleanSession = true,
                 std::chrono::seconds keepAlive = std::chrono::seconds{20});
    void disconnect();

    // v3 publish/subscribe (no reason codes/properties)
    void sendMessage(const std::string& topic,
                     const std::string& payload,
                     int qos = 1,
                     bool retained = false);

    void subscribe(const std::string& topic, int qos = 1);

    void setMessageHandler(MsgHandler handler) { userMessageHandler_ = std::move(handler); }

    mqtt::async_client& client() { return client_; }

private:
    // v3 callback interface
    class Callback : public virtual mqtt::callback {
    public:
        explicit Callback(MqttClient* owner) : owner_(owner) {}
        void message_arrived(mqtt::const_message_ptr msg) override;
        void connection_lost(const std::string& cause) override;
    private:
        MqttClient* owner_;
    };

    MsgHandler userMessageHandler_;
    mqtt::async_client client_;
    Callback cb_;
};


#endif

#endif // MQTTCLIENT_H
