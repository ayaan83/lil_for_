#include "util.h"

void printHex(uint8_t* buf, size_t size){
    for(int i=0;i<size;i++){
        Serial.print(buf[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

// 문자열 MAC 주소를 6바이트 HEX 값으로 변환하는 함수
void convertMacAddressToHex(const char* macStr, uint8_t* macHex) {
  unsigned int mac[6]; // 임시 배열로 unsigned int 사용 (sscanf에서 %x 사용시 필요)
  
  // MAC 주소 문자열 분석
  sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &macHex[0], &macHex[1], &macHex[2], &macHex[3], &macHex[4], &macHex[5]);
}

void toBigEndianFrom64toByte(uint64_t src, uint8_t* des){
    for(int i=7;i>=0;i--){
        des[i] = (uint8_t)((src >> (8*i))& 0xFF);
    }
}

void toBigEndianFrom16toByte(uint16_t src, uint8_t* des){
    for(int i=1;i>=0; i--){
        des[i] = (uint8_t)((src >> (8*i))& 0xFF);
    }
}