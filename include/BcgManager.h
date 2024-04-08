#include "def.h"

// BCG Value
#define UART_BAUDRATE_115200    115200

#define GPIO_BCG_UART_RX 32
#define GPIO_BCG_UART_TX 33

#define BCG_START_PACKET 127
#define BCG_PACKET_SIZE 24
#define BCG_DATA_COUNT_MAX 128
#define BCG_DATA_QUEUE_ONE_SIZE 13
#define BCG_DATA_QUEUE_DATA_COUNT 8
#define BCG_DATA_UDP_DATA_SEND_COUNT BCG_DATA_COUNT_MAX * BCG_DATA_QUEUE_DATA_COUNT
#define BCG_PACKET_SIZE_MESSEGE_QUEUE 1 + (BCG_DATA_QUEUE_ONE_SIZE * BCG_DATA_COUNT_MAX)

#define BCG_CMD_SIZE 4


typedef enum {
    BCG_START,
    BCG_STOP,
    BCG_FAST,
    BCG_SLOW,
    BCG_ANALYSIS_DIABLE,
    BCG_ANALISYS_ENABLE,
    BCG_CMD_END,
} bcg_cmd_t;

typedef enum{
    BCG_DATA_MARK = 0,
    BCG_DATA_PUD,
    BCG_DATA_PC,
    BCG_DATA_SPECTRAL_HEART,
    BCG_DATA_SPECTRAL_RESP,
    BCG_DATA_HR_VALIDITY,
    BCG_DATA_RR_VALIDITY,
    BCG_DATA_HR,
    BCG_DATA_RR,
    BCG_DATA_RMS_HEART,
    BCG_DATA_RMS_RESP,
    BCG_DATA_SIG_HEART,
    BCG_DATA_SIG_RESP,
    BCG_DATA_BCG_FILTERED_CH_H,
    BCG_DATA_BCG_FILTERED_CH_L,
    BCG_DATA_BCG_AMP_CH_H,
    BCG_DATA_BCG_AMP_CH_L,
    BCG_DATA_BCG_MOTION_X_CH_H,
    BCG_DATA_BCG_MOTION_X_CH_L,
    BCG_DATA_BCG_MOTION_Y_CH_H,
    BCG_DATA_BCG_MOTION_Y_CH_L,
    BCG_DATA_BCG_MOTION_Z_CH_H,
    BCG_DATA_BCG_MOTION_Z_CH_L,
}bcg_data_enum_t;

typedef enum{
    BCG_PUD_ORIGIN_HEART,
    BCG_PUD_ORIGIN_RESP,
    BCG_PUD_ANAL_EN,
    BCG_PUD_RESPSIGNAL_OK,
    BCG_PUD_HEARTSIGNAL_OK,
    BCG_PUD_BCG_OK,
    BCG_PUD_STREAM_MODE,
    BCG_PUD_COM_RESPONSE_DATA,
}bcg_pud_t;

typedef enum{
    BCG_DATA_TYPE_START,
    BCG_DATA_TYPE_16HZ_BASIC,
    BCG_DATA_TYPE_16HZ_WITH_IMU,
    BCG_DATA_TYPE_256HZ_BCG_RAW,
    BCG_DATA_TYPE_DATA_ALL,
    BCG_DATA_TYPE_END,
}bcg_send_data_type_t;

typedef enum{
    BCG_STREAM_TYPE_NOT_SET = -1,
    BCG_STREAM_TYPE_16HZ = 0,
    BCG_STREAM_TYPE_256HZ = 1,
}bcg_stream_type_t;

// bcg raw data type{
typedef struct {
    uint8_t hrValidity;
    uint8_t rrValidity;
    uint8_t hr;
    uint8_t rr;
} bcg_hr_data_t;

typedef struct {
    uint8_t motionX[2];
    uint8_t motionY[2];
    uint8_t motionZ[2];
} bcg_imu_data_t;
// }

typedef struct {
    bcg_stream_type_t isFastStream;
    uint8_t pc;
    bcg_hr_data_t HRData;
    uint8_t bcg[2];   
    bcg_imu_data_t imu;
}bcg_parse_data_t;

// bcg send data type {
typedef struct{
    uint8_t pc;
    bcg_hr_data_t HRData;
}bcg_basic_data_t;

typedef struct{
    uint8_t pc;
    bcg_hr_data_t HRData;
    bcg_imu_data_t imu;
}bcg_basic_with_imu_data_t;

typedef struct{
    uint8_t pc;
    uint8_t bcg[2];
}bcg_raw_data_only_t;

typedef struct{
    uint8_t pc;
    bcg_hr_data_t HRData;
    uint8_t bcg[2];
    bcg_imu_data_t imu;
}bcg_all_data_t;

// }

class BcgManager
{
public:
    BcgManager(HardwareSerial& sensorUart);
    static BcgManager* getInstance(HardwareSerial& sensorUart);
    
    void start();
    void stop();
    void defaultStart();
    void setFast();
    void setSlow();
    void setAnalysisDisable();
    void setAnalysisEnable();
    void set16Hz();
    void set256Hz();
    void read(uint8_t* readData, uint8_t* size);
    bool readOnePacket(uint8_t* readData);
    void getParsedData(uint8_t* src, bcg_parse_data_t* des);


private:
    HardwareSerial stream;
    static BcgManager* instance;

    uint8_t readOneByte();
    void conditionalDelay(unsigned long ms);
};