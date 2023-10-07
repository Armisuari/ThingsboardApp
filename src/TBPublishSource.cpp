#include "TBPublishSource.h"

TBPublishSource::TBPublishSource(uint32_t bufferSize) : _messageBuffSize(bufferSize), _receivingTaskHandle(NULL)
{
    
}

bool TBPublishSource::init(TaskHandle_t receivingTaskHandle)
{
    _receivingTaskHandle = receivingTaskHandle;
    _messageBuffHandle = xMessageBufferCreate(_messageBuffSize);
    return _messageBuffHandle != NULL;
}

size_t TBPublishSource::writeRawData(void *buff, size_t length)
{
    size_t res = xMessageBufferSend(_messageBuffHandle, buff, length, 0);
    if (res == length)
    {
        xTaskNotifyGive(_receivingTaskHandle);
    }
    return res;
}

size_t TBPublishSource::readRawData(void *buff, size_t length)
{
    return xMessageBufferReceive(_messageBuffHandle, buff, length, 0);
}

bool TBPublishSource::available()
{
    return (xMessageBufferIsEmpty(_messageBuffHandle) == pdFALSE);
}