#include "ModemTask.h"



void ModemTask(void *pvParameters){
    Serial.println("modem task start");
    ModemManager* modemManager = ModemManager::getInstance(Serial1);
    for(;;){
        char reciveData[DATA_QUEUE_SIZE] = {0,};
        if(xQueueReceive(dataQueueBCG, reciveData, portMAX_DELAY) == pdTRUE){
            messege_queue_data_t* pReceiveData = (messege_queue_data_t*)reciveData;
            DEBUG_PRINT("length : " + String(pReceiveData->length));
            if(!modemManager->sendUdp(UDP_SERVER_IP, UDP_SERVER_PORT, (uint8_t*)pReceiveData->data, pReceiveData->length)){
                Serial.println("send UDP is failed!!");
                Serial.println("modem restart");
                modemManager->modemRestart();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}