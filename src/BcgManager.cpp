#include "BcgManager.h"

BcgManager* BcgManager::instance = nullptr;

BcgManager::BcgManager(HardwareSerial& sensorUart) : stream(sensorUart){
    stream.begin(UART_BAUDRATE_115200, SERIAL_8N1, GPIO_BCG_UART_RX, GPIO_BCG_UART_TX);
}

BcgManager* BcgManager::getInstance(HardwareSerial& stream){
    Serial.println("get Bcg instance");
    if(!instance){
        instance = new BcgManager(stream);
    }
    return instance;
}

void BcgManager::defaultStart(){
    Serial.println("BCG Start");
    pinMode(12, OUTPUT); //boost temp
    digitalWrite(12, HIGH);
    conditionalDelay(2000);
    set16Hz();
    start();
}

void BcgManager::start(){
    uint8_t cmd[] = {1, 0, 2, 0};
    stream.write(cmd, BCG_CMD_SIZE);
}

void BcgManager::stop(){
    uint8_t cmd[] = {1, 0, 1, 0};
    stream.write(cmd, BCG_CMD_SIZE);
}

void BcgManager::setFast(){
    uint8_t cmd[] = {1, 0, 5, 0};
    stream.write(cmd, BCG_CMD_SIZE);
    conditionalDelay(100);
}

void BcgManager::setSlow(){
    uint8_t cmd[] = {1, 0, 6, 0};
    stream.write(cmd, BCG_CMD_SIZE);
    conditionalDelay(100);
}

void BcgManager::setAnalysisDisable(){
    uint8_t cmd[] = {1, 0, 9, 0};
    stream.write(cmd, BCG_CMD_SIZE);
    conditionalDelay(100);
}

void BcgManager::setAnalysisEnable(){
    uint8_t cmd[] = {1, 0, 10, 0};
    stream.write(cmd, BCG_CMD_SIZE);
}

void BcgManager::set16Hz(){
    setSlow();
    setAnalysisEnable();
}

void BcgManager::set256Hz(){
    setAnalysisDisable();
    setFast();
}

void BcgManager::read(uint8_t* readData, uint8_t* size){
    Serial.println("read");
    while(stream.available()){
        *readData++ = stream.read();
        (*size)++;
    }   
}

bool BcgManager::readOnePacket(uint8_t* readData){
    if(!stream.available()) return false;

    uint8_t byte = stream.read();
    uint8_t index = 0;
    if(byte != BCG_START_PACKET) return false;

    readData[index++] = byte;
    for(int i=0;i<BCG_PACKET_SIZE-1;i++){
        readData[index++] = stream.read();
    }
    return true;
}

void BcgManager::getParsedData(uint8_t* src, bcg_parse_data_t* des){
    des->isFastStream = src[BCG_DATA_PUD] & (1 << BCG_PUD_STREAM_MODE) ? BCG_STREAM_TYPE_256HZ:BCG_STREAM_TYPE_16HZ;
    // for(int i=7;i>=0;i--){
    //     Serial.print(src[BCG_DATA_PUD] & (1 << i) ? 1:0);
    //     Serial.print(" ");
    // }
    // Serial.println();
    des->pc = src[BCG_DATA_PC];
    des->HRData.hrValidity = src[BCG_DATA_HR_VALIDITY];
    des->HRData.hr = src[BCG_DATA_HR];
    des->HRData.rrValidity = src[BCG_DATA_RR_VALIDITY];
    des->HRData.rr = src[BCG_DATA_RR];
    des->bcg[0] = src[BCG_DATA_BCG_FILTERED_CH_H];
    des->bcg[1] = src[BCG_DATA_BCG_FILTERED_CH_L];
    des->imu.motionX[0] = src[BCG_DATA_BCG_MOTION_X_CH_H];
    des->imu.motionX[1] = src[BCG_DATA_BCG_MOTION_X_CH_L];
    des->imu.motionY[0] = src[BCG_DATA_BCG_MOTION_Y_CH_H];
    des->imu.motionY[1] = src[BCG_DATA_BCG_MOTION_Y_CH_L];
    des->imu.motionZ[0] = src[BCG_DATA_BCG_MOTION_Z_CH_H];
    des->imu.motionZ[1] = src[BCG_DATA_BCG_MOTION_Z_CH_L];
}

void BcgManager::conditionalDelay(unsigned long ms){
    if (xTaskGetCurrentTaskHandle() != NULL){
        vTaskDelay(pdMS_TO_TICKS(ms));
    }else{
        delay(ms);
    }
}