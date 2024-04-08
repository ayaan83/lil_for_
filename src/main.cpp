#include "main.h"
#include "DataQueue.h"
#include "ModemManager.h"

QueueHandle_t dataQueueBCG;
QueueHandle_t dataQueueToModem;

// MainTask mainTask;
// BcgManager bcgManager;
// ModemManager modemManager;

void setup(){
    Serial.begin(UART_BAUDRATE_115200);
    dataQueueBCG = xQueueCreate(DATA_QUEUE_LENGTH, 1024);
    dataQueueToModem = xQueueCreate(DATA_QUEUE_LENGTH, sizeof(uint64_t)); // TO DO 사이즈 수정 필요

    ModemManager* modemManager = ModemManager::getInstance(Serial1);

    // MainTask의 static taskWrapper 정의 필요
    // xTaskCreate(
    //     MainTask::taskWrapper,
    //     "MainTask",
    //     10000,
    //     static_cast<void*>(&mainTask),
    //     PRIORITY_MAIN_TASK, 
    //     NULL
    // );

    xTaskCreate(
        ModemTask,
        "ModemTask",
        10000,
        NULL,
        PRIORITY_BCG_TASK, 
        NULL
    );

    xTaskCreate(
        BcgTask,
        "BcgTask",
        10000,
        NULL,
        // PRIORITY_BCG_TASK, 
        2, 
        NULL
    );
}

void loop(){
}
