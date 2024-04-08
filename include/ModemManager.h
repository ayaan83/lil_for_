#pragma once
#include "def.h"

#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

#include <Ticker.h>
#include <TinyGsmClient.h>

#define LILYGO_T_CALL_A7670_V1_0 
// #define LILYGO_T_A7670 

#if defined(LILYGO_T_A7670)
// MODEM GPIO
#define GPIO_OUTPUT_MODEM_RESET 5
#define GPIO_OUTPUT_MODEM_POWER 4
#define GPIO_OUTPUT_MODEM_DTR   25
#define GPIO_OUTPUT_MODEM_RI    33
#define GPIO_OUTPUT_MODEM_TX    26
#define GPIO_OUTPUT_MODEM_RX    27
#define GPIO_OUTPUT_MODEM_ENABLE    12
#define GPIO_OUTPUT_BATTERY_VOLTAGE 35

#define MODEM_RESET_LEVEL                   HIGH

#elif defined(LILYGO_T_CALL_A7670_V1_0)

// MODEM GPIO
#define GPIO_OUTPUT_MODEM_RESET 27
#define GPIO_OUTPUT_MODEM_POWER 4
#define GPIO_OUTPUT_MODEM_DTR   14
#define GPIO_OUTPUT_MODEM_RI    13
#define GPIO_OUTPUT_MODEM_TX    26
#define GPIO_OUTPUT_MODEM_RX    25
#define GPIO_OUTPUT_MODEM_ENABLE    12

#define MODEM_RESET_LEVEL                   LOW

#define MODEM_GPS_ENABLE_GPIO               (-1)
#elif defined (LILYGO_T_CALL_A7670_V1_1)

// MODEM GPIO
#define GPIO_OUTPUT_MODEM_RESET 5
#define GPIO_OUTPUT_MODEM_POWER 4
#define GPIO_OUTPUT_MODEM_DTR   32
#define GPIO_OUTPUT_MODEM_RI    33
#define GPIO_OUTPUT_MODEM_TX    27
#define GPIO_OUTPUT_MODEM_RX    26
#define GPIO_OUTPUT_MODEM_ENABLE    12

#define MODEM_RESET_LEVEL                   LOW

#define MODEM_GPS_ENABLE_GPIO               (-1)
#else
#error "Use ArduinoIDE, please open the macro definition corresponding to the board above <utilities.h>"
#endif

#define UART_BAUDRATE_115200    115200
#define GSM_PIN ""

#define APN "internet.lguplus.co.kr"
#define GPRS_USER   ""
#define GPRS_PASS   ""

#define NTP_SERVER "pool.ntp.org"
#define KOREA_TIMEZONE 9
#define NTP_TIMEZONE 4 * 9

typedef enum{
    MODEM_STATE_START,
    MODEM_STATE_SET_PIN,
    MODEM_STATE_RESET,
    MODEM_STATE_INIT,
    MODEM_STATE_CONNECTED,
    MODEM_STATE_IN_USE,
    MODEM_STATE_ERROR,
    MODEM_STATE_END,
} modemState_t;


class ModemManager{
public:
    ModemManager(HardwareSerial& modemUart);
    static ModemManager* getInstance(HardwareSerial& modemUart);

    void modemStart();
    void modemRestart();
    void getNetworkTime(int* year, int* month, int* day, int* hour, int* minute, int* second);
    uint64_t getUnixLocalTime();
    bool getGps(float* lat2, float* lon2, float* speed2, float* alt2, int* vsat2, int* usat2, float* accuracy2);
    bool getLbs(float* lat, float* lon, float* accuracy);
    bool sendUdp(String serverIP, uint16_t serverPort, uint8_t* sendData, size_t dataSize);

private:
    static ModemManager* instance;

    modemState_t state;

    TinyGsm* modem;

    void setModemPin();
    void modemReset();
    bool modemInit();
    bool modemCheck();
    void conditionalDelay(unsigned long ms);
};