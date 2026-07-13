#include <drogon/drogon.h>

int main()
{
    drogon::app()
        .setLogLevel(trantor::Logger::kDebug)
        .loadConfigFile("../config.json")
        .addListener("0.0.0.0", 8080)
        .setThreadNum(4)
        .run();
    return 0;
}
