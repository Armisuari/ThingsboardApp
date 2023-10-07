#pragma once

#include <WString.h>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/message_buffer.h"
#include "freertos/task.h"

class TBPublishSource
{
    friend class TBHandler;

public:
    TBPublishSource(uint32_t bufferSize);
    size_t writeRawData(void *buff, size_t length);

protected:
    bool init(TaskHandle_t receivingTaskHandle);
    size_t readRawData(void *buff, size_t length);
    bool available();
    virtual std::string readPayload() = 0;
    virtual size_t readPayload(uint8_t *buff, size_t length) = 0;

private:
    MessageBufferHandle_t _messageBuffHandle;
    size_t _messageBuffSize;
    TaskHandle_t _receivingTaskHandle;
};