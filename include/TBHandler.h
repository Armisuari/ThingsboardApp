#pragma once

#include <WiFiClientSecure.h>
#include <ThingsBoard.h>
#include <ArduinoJson.h>
#include <vector>
#include <WiFi.h>
#include "config.h"
#include "TBPublishSource.h"

class TBHandler
{
public:
    TBHandler(String token);
    void setup(String addrServer, const uint16_t port, String clientID);
    void setup(String addrServer, const uint16_t port, size_t buff);
    bool addPublishSource(TBPublishSource *pubSource);
    void setBufferSize(size_t buff);

private:
    String _token;
    ThingsBoard tb;
    TaskHandle_t _taskHandle;
    std::vector<TBPublishSource *> _publishSources;
    WiFiClient espClient;
    String _clientID;
    String _addrServer;
    uint16_t _port;

    static void _staticTaskFunc(void *pvParam);
    void _taskFunc();
};