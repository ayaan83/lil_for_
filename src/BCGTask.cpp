#include "BcgTask.h"
#include "util.h"

static bcg_task_data_t bcgTaskData;
static messege_queue_data_t sendData = {0,};



void BcgTask(void *pvParameters){
    DEBUG_PRINT("BCG Task Start!!");
    initData();    
    bcgTaskData.previousStreamType = BCG_STREAM_TYPE_NOT_SET;
    BcgManager* bcgManager = BcgManager::getInstance(Serial2);
    bcgManager->defaultStart();
    for(;;){
        processBcgData(bcgManager);
        if(Serial.available()){
            char cmd = Serial.read();
            
            if(cmd == '1'){
                
                bcgManager->set16Hz();
            }else if(cmd == '2'){
                bcgManager->set256Hz();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(3));
    }
}

void initData(){
    ModemManager* modemManager = ModemManager::getInstance(Serial1);
    convertMacAddressToHex(WiFi.macAddress().c_str(), bcgTaskData.wifiMacAddress);
    bcgTaskData.unixTime = modemManager->getUnixLocalTime();
    modemManager->modemStart();
}

void processBcgData(BcgManager* bcgManager){
    // DEBUG_PRINT("processBcgData");
    uint8_t buf[BCG_PACKET_SIZE]={0,};
    if(!bcgManager->readOnePacket(buf)) return;
    bcg_parse_data_t bcgData;
    bcgManager->getParsedData(buf, &bcgData);
    mergeBcgData(&bcgData);
}

void mergeBcgData(bcg_parse_data_t* bcgData){
    if(bcgTaskData.previousStreamType != bcgData->isFastStream){
        DEBUG_PRINT(String(bcgData->isFastStream) + ", " + String(bcgTaskData.previousStreamType));

        bcgTaskData.previousStreamType = bcgData->isFastStream;
        mergeDataReset();
    }
    if(bcgData->isFastStream == BCG_STREAM_TYPE_256HZ){
        mergeBcgData256Hz(bcgData);
    }
    else if(bcgData->isFastStream == BCG_STREAM_TYPE_16HZ){
        mergeBcgData16Hz(bcgData);
    }
    else{
        Serial.println("Not Implement Stream level!!");
    }

}

void mergeDataReset(){
    DEBUG_PRINT("mergeDataReset : count, index : " + String(bcgTaskData.bcgDataCount));
    memset(&sendData, 0, sizeof(messege_queue_data_t));
    bcgTaskData.bcgDataCount = 0;
}

void mergeBcgData256Hz(bcg_parse_data_t* bcgData){
    // DEBUG_PRINT("mergeBcgData256Hz");
    bcg_raw_only_send_data_t* bcgRawOnly = (bcg_raw_only_send_data_t* )sendData.data;
    bcgRawOnly->data[bcgTaskData.bcgDataCount].pc = bcgData->pc;
    bcgRawOnly->data[bcgTaskData.bcgDataCount].bcg[0] = bcgData->bcg[0];
    bcgRawOnly->data[bcgTaskData.bcgDataCount].bcg[1] = bcgData->bcg[1];
    bcgTaskData.bcgDataCount++;
    if(bcgTaskData.bcgDataCount == BCG_COUNT_256){
    // if(bcgTaskData.bcgDataCount == 1){
        setHeaderAndTail(&bcgRawOnly->header, bcgRawOnly->etx);
        bcgRawOnly->dataType = BCG_DATA_TYPE_256HZ_BCG_RAW;
        sendData.length = sizeof(bcg_raw_only_send_data_t);
        DEBUG_PRINT("256hz : bcgcount : " + String(bcgTaskData.bcgDataCount));
        xQueueSend(dataQueueBCG, &sendData, sizeof(messege_queue_data_t));
        mergeDataReset();
    }
}



void mergeBcgData16Hz(bcg_parse_data_t* bcgData){
    // DEBUG_PRINT("mergeBcgData16Hz");
    bcg_basic_with_imu_send_data_t *bcgBasicWithImu = (bcg_basic_with_imu_send_data_t*)sendData.data;
    bcgBasicWithImu->data[bcgTaskData.bcgDataCount].pc = bcgData->pc;
    bcgBasicWithImu->data[bcgTaskData.bcgDataCount].HRData.hr = bcgData->HRData.hr;
    bcgBasicWithImu->data[bcgTaskData.bcgDataCount].HRData.hrValidity = bcgData->HRData.hrValidity;
    bcgBasicWithImu->data[bcgTaskData.bcgDataCount].HRData.rr = bcgData->HRData.rr;
    bcgBasicWithImu->data[bcgTaskData.bcgDataCount].HRData.rrValidity = bcgData->HRData.rrValidity;
    bcgBasicWithImu->data[bcgTaskData.bcgDataCount].imu.motionX[0] = bcgData->imu.motionX[0];
    bcgBasicWithImu->data[bcgTaskData.bcgDataCount].imu.motionX[1] = bcgData->imu.motionX[1];
    bcgBasicWithImu->data[bcgTaskData.bcgDataCount].imu.motionY[0] = bcgData->imu.motionY[0];
    bcgBasicWithImu->data[bcgTaskData.bcgDataCount].imu.motionY[1] = bcgData->imu.motionY[1];
    bcgBasicWithImu->data[bcgTaskData.bcgDataCount].imu.motionZ[0] = bcgData->imu.motionZ[0];
    bcgBasicWithImu->data[bcgTaskData.bcgDataCount].imu.motionZ[1] = bcgData->imu.motionZ[1];
    bcgTaskData.bcgDataCount++;
    if(bcgTaskData.bcgDataCount == BCG_COUNT_64){
    // if(bcgTaskData.bcgDataCount == 1){
        setHeaderAndTail(&bcgBasicWithImu->header, bcgBasicWithImu->etx);
        bcgBasicWithImu->dataType = BCG_DATA_TYPE_16HZ_WITH_IMU;
        sendData.length = sizeof(bcg_basic_with_imu_send_data_t);
        DEBUG_PRINT("16 : " + String(bcgTaskData.bcgDataCount));
        xQueueSend(dataQueueBCG, &sendData, sizeof(messege_queue_data_t));
        mergeDataReset();
    }
}

void setHeaderAndTail(bcg_send_header_t* header, uint8_t* tail){
    header->stx[0] = 'D';
    header->stx[1] = 'B';
    memcpy(header->wifiMacAddress, bcgTaskData.wifiMacAddress, sizeof(header->wifiMacAddress));
    toBigEndianFrom64toByte(bcgTaskData.unixTime + millis(), header->unixTime);
    getBatteryVoltage(header->batteryVoltage);
    tail[0] = 'B';
    tail[1] = 'D';
}

void getBatteryVoltage(uint8_t* voltage){
    uint16_t v = 3700;
    toBigEndianFrom16toByte(v, voltage);
}