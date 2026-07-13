#include "WeatherStream.h"
#include <json/json.h>
#include <drogon/PubSubService.h>

std::mutex WeatherStream::subscriptionsMutex_;
std::map<WebSocketConnectionPtr, WeatherStream::Subscription> WeatherStream::subscriptions_;
PubSubService<std::string> WeatherStream::pubSub_;

void WeatherStream::handleNewMessage(const WebSocketConnectionPtr &wsConnPtr,
                                     std::string &&message,
                                     const WebSocketMessageType &type)
{
    if (type != WebSocketMessageType::Text)
    {
        return;
    }

    // Parse incoming message as JSON
    Json::Value jsonMsg;
    Json::CharReaderBuilder reader;
    std::istringstream stream(message);
    std::string errors;
    if (!Json::parseFromStream(reader, stream, &jsonMsg, &errors))
    {
        Json::Value errResp;
        errResp["error"] = "Invalid JSON: " + errors;
        Json::StreamWriterBuilder writer;
        wsConnPtr->send(Json::writeString(writer, errResp));
        return;
    }

    // Client can send {"action": "subscribe", "city": "London"}
    // or {"action": "unsubscribe"}
    if (jsonMsg.isMember("action"))
    {
        std::string action = jsonMsg["action"].asString();

        if (action == "subscribe" && jsonMsg.isMember("city"))
        {
            std::string newTopic = jsonMsg["city"].asString();

            // Unsubscribe from old topic
            {
                std::lock_guard<std::mutex> lock(subscriptionsMutex_);
                auto it = subscriptions_.find(wsConnPtr);
                if (it != subscriptions_.end())
                {
                    pubSub_.unsubscribe(it->second.topic, it->second.subscriptionId);
                    subscriptions_.erase(it);
                }
            }

            // Subscribe to new topic
            auto subId = pubSub_.subscribe(
                newTopic,
                [wsConnPtr](const std::string &topic, const std::string &msg) {
                    wsConnPtr->send(msg);
                });

            {
                std::lock_guard<std::mutex> lock(subscriptionsMutex_);
                subscriptions_[wsConnPtr] = {newTopic, subId};
            }

            Json::Value confirm;
            confirm["status"] = "subscribed";
            confirm["city"] = newTopic;
            Json::StreamWriterBuilder writer;
            wsConnPtr->send(Json::writeString(writer, confirm));
        }
        else if (action == "unsubscribe")
        {
            std::lock_guard<std::mutex> lock(subscriptionsMutex_);
            auto it = subscriptions_.find(wsConnPtr);
            if (it != subscriptions_.end())
            {
                pubSub_.unsubscribe(it->second.topic, it->second.subscriptionId);
                subscriptions_.erase(it);
            }

            Json::Value confirm;
            confirm["status"] = "unsubscribed";
            Json::StreamWriterBuilder writer;
            wsConnPtr->send(Json::writeString(writer, confirm));
        }
    }
}

void WeatherStream::handleNewConnection(const HttpRequestPtr &req,
                                        const WebSocketConnectionPtr &conn)
{
    Json::Value welcome;
    welcome["status"] = "connected";
    welcome["message"] = "Send {\"action\":\"subscribe\",\"city\":\"CityName\"} to subscribe to weather updates";
    Json::StreamWriterBuilder writer;
    conn->send(Json::writeString(writer, welcome));
}

void WeatherStream::handleConnectionClosed(const WebSocketConnectionPtr &conn)
{
    std::lock_guard<std::mutex> lock(subscriptionsMutex_);
    auto it = subscriptions_.find(conn);
    if (it != subscriptions_.end())
    {
        pubSub_.unsubscribe(it->second.topic, it->second.subscriptionId);
        subscriptions_.erase(it);
    }
}
