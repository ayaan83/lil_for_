#include "def.h"
#include "BcgManager.h"
#include "ModemManager.h"
#include "WiFi.h"
#include "DataQueue.h"

#define BCG_MERGE_COUNT 64
#define BCG_SEND_BUF_SIZE 1024

#define BCG_COUNT_256 256
#define BCG_COUNT_64 64

typedef struct{
    bcg_stream_type_t previousStreamType;
    uint16_t bcgDataCount;
    uint8_t wifiMacAddress[6];
    uint64_t unixTime;
    uint8_t batteryVoltage[2];
    bcg_send_data_type_t dataType;
    uint8_t data[BCG_SEND_BUF_SIZE];
}bcg_task_data_t;

typedef struct{
    uint8_t stx[2];
    uint8_t wifiMacAddress[6];
    uint8_t unixTime[8];
    uint8_t batteryVoltage[2];
}bcg_send_header_t;

typedef struct{
    bcg_send_header_t header;
    uint8_t dataType;
    bcg_basic_with_imu_data_t data[64];
    uint8_t etx[2];
}bcg_basic_with_imu_send_data_t;

typedef struct{
    bcg_send_header_t header;
    uint8_t dataType;
    bcg_raw_data_only_t data[256];
    uint8_t etx[2];
}bcg_raw_only_send_data_t;


void BcgTask(void *pvParameters);
void initData();
void processBcgData(BcgManager* bcgManager);
void mergeBcgData(bcg_parse_data_t* bcgData);
void mergeDataReset();
void mergeBcgData256Hz(bcg_parse_data_t* bcgData);
void mergeBcgData16Hz(bcg_parse_data_t* bcgData);
void setHeaderAndTail(bcg_send_header_t* header, uint8_t* tail);
void getBatteryVoltage(uint8_t* voltage);