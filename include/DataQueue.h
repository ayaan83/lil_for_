#pragma once
#include "def.h"

#define DATA_QUEUE_SIZE 1024
#define DATA_QUEUE_LENGTH 20

extern QueueHandle_t dataQueueBCG;

typedef struct{
    size_t length;
    uint8_t data[1024-2];
}messege_queue_data_t;