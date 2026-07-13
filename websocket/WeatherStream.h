#pragma once

#include <drogon/WebSocketController.h>
#include <drogon/PubSubService.h>
#include <drogon/drogon.h>
#include <set>
#include <mutex>

using namespace drogon;

class WeatherStream : public drogon::WebSocketController<WeatherStream>
{
  public:
    void handleNewMessage(const WebSocketConnectionPtr &wsConnPtr,
                          std::string &&message,
                          const WebSocketMessageType &type) override;

    void handleNewConnection(const HttpRequestPtr &req,
                             const WebSocketConnectionPtr &conn) override;

    void handleConnectionClosed(const WebSocketConnectionPtr &conn) override;

    WS_PATH_LIST_BEGIN
    WS_PATH_ADD("/ws/weather/{1}", Get);
    WS_PATH_LIST_END

  private:
    struct Subscription
    {
        std::string topic;
        size_t subscriptionId;
    };

    static std::mutex subscriptionsMutex_;
    static std::map<WebSocketConnectionPtr, Subscription> subscriptions_;
    static PubSubService<std::string> pubSub_;
};
