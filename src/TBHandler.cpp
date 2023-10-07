#include "TBHandler.h"

#define TBhandler_tag "TBHANDLER"

TBHandler::TBHandler(String token) : _token(token)
{
    tb.setClient(espClient);
}

void TBHandler::setBufferSize(size_t buff)
{
    tb.setBufferSize(buff);
}

void TBHandler::setup(String addrServer, const uint16_t port, size_t buff)
{
    setBufferSize(buff);
    setup(addrServer, port, "default");
}

void TBHandler::setup(String addrServer, const uint16_t port, String clientID)
{
    _addrServer = addrServer;
    _port = port;
    _clientID = clientID;

    // create task for this handler
    xTaskCreate(&TBHandler::_staticTaskFunc,
                "ThingsBoard Handler",
                4096,
                this,
                1,
                &_taskHandle);
}

/* STATIC */ void TBHandler::_staticTaskFunc(void *pvParam)
{
    TBHandler *handler = reinterpret_cast<TBHandler *>(pvParam);
    handler->_taskFunc();
}

void TBHandler::_taskFunc()
{
    ESP_LOGD(TBhandler_tag, "TB Handler Task Started");

    while (1)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            ESP_LOGD(TBhandler_tag, "Waiting wifi connection...");
            // client.disconnect();
            tb.disconnect();

            while (WiFi.status() != WL_CONNECTED)
            {
                // TODO: find mechanism to block task and immediately resume task if wifi connected
                delay(1000);
            }
        }
        else if (WiFi.status() == WL_CONNECTED && !tb.connected())
        {
            ESP_LOGD(TBhandler_tag, "Connecting to ThingsBoard Server...");
            tb.connect(_addrServer.c_str(), _token.c_str(), _port, _clientID.c_str());
            ESP_LOGI(TBhandler_tag, "Connected to TB Server !");
        }
        else
        {
            tb.loop();

            uint32_t newPubAvail = ulTaskNotifyTake(pdFALSE, pdMS_TO_TICKS(1000));
            if (newPubAvail > 0)
            {
                for (auto pubsource : _publishSources)
                {

                    // Serial.println("Checking pubsource ready...");
                    if (!pubsource->available())
                    {
                        continue;
                    }

                    // Serial.println("Reading payload...");
                    std::string payload = pubsource->readPayload();
                    if (payload.length() == 0)
                    {
                        continue;
                    }

                    // Serial.println("send telemetry payload...");

                    bool res = tb.sendTelemetryJson(payload.c_str());
                    ESP_LOGD(TBhandler_tag, "Sending Telemetry data... | msg: %s", payload.c_str());
                    if (!res)
                    {
                        ESP_LOGE(TBhandler_tag, "Publish Fail");
                        break;
                    }
                }
            }
        }
    }
    vTaskDelete(NULL);
}

bool TBHandler::addPublishSource(TBPublishSource *pubSource)
{
    ESP_LOGD(TBhandler_tag, "Adding Publish Source...");

    if (!pubSource)
    {
        ESP_LOGE(TBhandler_tag, "Publish source not ready...");
        return false;
    }

    for (size_t i = 0; i < _publishSources.size(); i++)
    {
        if (_publishSources[i] == pubSource)
        {
            ESP_LOGE(TBhandler_tag, "Publish source size not match...");
            return false;
        }
    }

    if (!pubSource->init(_taskHandle))
    {
        ESP_LOGE(TBhandler_tag, "Publish source task not initialized...");
        return false;
    }
    _publishSources.push_back(pubSource);

    ESP_LOGD(TBhandler_tag, "Publish source was added");

    return true;
}