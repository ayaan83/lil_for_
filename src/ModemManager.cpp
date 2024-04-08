#include "ModemManager.h"

ModemManager* ModemManager::instance = nullptr;

ModemManager::ModemManager(HardwareSerial& modemUart) : state(MODEM_STATE_START)
{
    modemUart.begin(UART_BAUDRATE_115200, SERIAL_8N1, GPIO_OUTPUT_MODEM_RX, GPIO_OUTPUT_MODEM_TX);
    modem = new TinyGsm(modemUart);
}

ModemManager* ModemManager::getInstance(HardwareSerial& modemUart)
{
    DEBUG_PRINT("get Modem instance");
    if(!instance){
        instance = new ModemManager(modemUart);
    }
    return instance;
}

void ModemManager::modemStart(){
    setModemPin();
    modemReset();
    modemInit();
}

void ModemManager::modemRestart(){
    modemReset();
    modemInit();
}

void ModemManager::setModemPin(){
    Serial.println("Modem Pin set!!");
    state = MODEM_STATE_SET_PIN;
    pinMode(GPIO_OUTPUT_MODEM_RESET, OUTPUT);
    pinMode(GPIO_OUTPUT_MODEM_POWER, OUTPUT);
}

void ModemManager::modemReset(){
    Serial.println("Modem Reset!!");
    state = MODEM_STATE_RESET;
    digitalWrite(GPIO_OUTPUT_MODEM_RESET, !MODEM_RESET_LEVEL); conditionalDelay(100);
    digitalWrite(GPIO_OUTPUT_MODEM_RESET, MODEM_RESET_LEVEL); conditionalDelay(3000);
    digitalWrite(GPIO_OUTPUT_MODEM_RESET, !MODEM_RESET_LEVEL);

    while(!modem->testAT(1000)){
        digitalWrite(GPIO_OUTPUT_MODEM_POWER, LOW); conditionalDelay(100);
        digitalWrite(GPIO_OUTPUT_MODEM_POWER, HIGH); conditionalDelay(1000);
        digitalWrite(GPIO_OUTPUT_MODEM_POWER, LOW);
    }
}

bool ModemManager::modemInit(){
    Serial.println("Initializing modem...");
    state = MODEM_STATE_INIT;
    if (!modem->init()) {
        Serial.println("Failed to initialize modem");
        return false;
    }
    String name = modem->getModemName();
    Serial.println("Modem Name:" + name);
    String modemInfo = modem->getModemInfo();
    Serial.println("Modem Info:" + modemInfo);
#if TINY_GSM_USE_GPRS
    // Unlock your SIM card with a PIN if needed
    if (GSM_PIN && modem->getSimStatus() != 3) {
        modem->simUnlock(GSM_PIN);
    }
#endif

#if TINY_GSM_USE_WIFI
        // Wifi connection parameters must be set before waiting for the network
    Serial.print(F("Setting SSID/password..."));
    if (!modem->networkConnect(wifiSSID, wifiPass)) {
        Serial.println(" fail");
        conditionalDelay(10000);
        return false;
    }
    Serial.println(" success");
#endif
    if (!modem->waitForNetwork()) {
        Serial.println("failt to wait for network");
        conditionalDelay(10000);
        return false;
    }
    Serial.println(" success");
    if (modem->isNetworkConnected()) {
        Serial.println("## Network connected ##");
    }
#if TINY_GSM_USE_GPRS
    // GPRS connection parameters are usually set after network registration
    Serial.print(F("Connecting to "));
    Serial.print(APN);
    if (!modem->gprsConnect(APN, GPRS_USER, GPRS_PASS)) {
        Serial.println(" fail");
        conditionalDelay(10000);
        return false;
    }
    Serial.println(" success");
    if (!modem->isGprsConnected()) {
        Serial.println("fail to GPRS connecte");
        return false;
    }
    Serial.println("GPRS connected");
    state = MODEM_STATE_CONNECTED;
#endif
    return true;
}

void ModemManager::getNetworkTime(int* year, int* month, int* day, int* hour, int* minute, int* second){
    float timezone;
    state = MODEM_STATE_IN_USE;
    modem->NTPServerSync(NTP_SERVER, NTP_TIMEZONE);
    modem->getNetworkTime(year, month, day, hour, minute, second, &timezone);
    Serial.println(String(*year) + "-" + String(*month) + "-" + String(*day) + " " + String(*hour) + ":" + String(*minute) + ":" + String(*second));
    state = MODEM_STATE_CONNECTED;
}

uint64_t ModemManager::getUnixLocalTime(){
    int year, month, day, hour, minute, second;
    float timezone;
    getNetworkTime(&year, &month, &day, &hour, &minute, &second);

    struct tm timeinfo = {0,};

    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;

    time_t epochTime = mktime(&timeinfo);
    return ((uint64_t)epochTime * 1000) - millis();
}

bool ModemManager::getGps(float* lat2, float* lon2, float* speed2, float* alt2, int* vsat2, int* usat2, float* accuracy2){
    int   year2     = 0;
    int   month2    = 0;
    int   day2      = 0;
    int   hour2     = 0;
    int   min2      = 0;
    int   sec2      = 0;
    uint8_t    fixMode   = 0;
    bool rValue = false;

    Serial.println("Requesting current GPS/GNSS/GLONASS location");
    if (modem->getGPS(&fixMode, lat2, lon2, speed2, alt2, vsat2, usat2, accuracy2,
                        &year2, &month2, &day2, &hour2, &min2, &sec2)) {
        rValue = true;
        Serial.print("FixMode:"); Serial.println(fixMode);

        Serial.print("Year:"); Serial.print(year2);
        Serial.print("\tMonth:"); Serial.print(month2);
        Serial.print("\tDay:"); Serial.println(day2);

        Serial.print("Hour:"); Serial.print(hour2);
        Serial.print("\tMinute:"); Serial.print(min2);
        Serial.print("\tSecond:"); Serial.println(sec2);
    } else {
        Serial.println("Couldn't get GPS/GNSS/GLONASS location");
    }
    Serial.println("Retrieving GPS/GNSS/GLONASS location again as a string");
    String gps_raw = modem->getGPSraw();
    Serial.print("GPS/GNSS Based Location String:");
    Serial.println(gps_raw);
    Serial.println("Disabling GPS");

    modem->disableGPS();
    return rValue;
}

bool ModemManager::getLbs(float* lat, float* lon, float* accuracy){
    int   year     = 0;
    int   month    = 0;
    int   day      = 0;
    int   hour     = 0;
    int   min      = 0;
    int   sec      = 0;
    bool rValue = false;

    Serial.println("Requesting current GSM location");
    if (modem->getGsmLocation(lat, lon, accuracy, &year, &month, &day, &hour,
                                &min, &sec)) {
        rValue = true;
        Serial.print("Year:"); Serial.print(year);
        Serial.print("\tMonth:"); Serial.print(month);
        Serial.print("\tDay:"); Serial.println(day);
        Serial.print("Hour:"); Serial.print(hour);
        Serial.print("\tMinute:"); Serial.print(min);
        Serial.print("\tSecond:"); Serial.println(sec);
    } else {
        DBG("Couldn't get GSM location, retrying in 15s.");
    }
    Serial.println("Retrieving GSM location again as a string");
    String location = modem->getGsmLocation();
    Serial.print("GSM Based Location String:");
    Serial.println(location);
    return rValue;
}

bool ModemManager::sendUdp(String serverIP, uint16_t serverPort, uint8_t* sendData, size_t dataSize){
    if(state != MODEM_STATE_CONNECTED) return false;
    int timeout_ms = 1000;
    modem->sendAT("+CIPOPEN=1,\"UDP\",,,5000");
    if (modem->waitResponse(timeout_ms, GF(GSM_NL "OK")) != 1) { return false; }
    modem->sendAT("+CIPSEND=1,", String(dataSize), ",\"", serverIP, "\",", serverPort);
    if (modem->waitResponse(GF(">")) != 1) return false;
    modem->stream.write(sendData, dataSize);
    modem->stream.write(reinterpret_cast<const uint8_t*>(GF(GSM_NL)), 2);
    modem->stream.flush();
    if(modem->waitResponse(timeout_ms, GF("OK")) != 1) return false;
    modem->sendAT(GF("+CIPCLOSE=1"));
    if (modem->waitResponse(timeout_ms, GF("+CIPCLOSE: 1,0")) != 1) return false;
    if (modem->waitResponse(timeout_ms, GF("OK")) != 1) return false;
    return true;
}

void ModemManager::conditionalDelay(unsigned long ms){
    if (xTaskGetCurrentTaskHandle() != NULL){
        vTaskDelay(pdMS_TO_TICKS(ms));
    }else{
        delay(ms);
    }
}