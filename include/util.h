#pragma once
#include "def.h"

void printHex(uint8_t* buf, size_t size);
void convertMacAddressToHex(const char* macStr, uint8_t* macHex);
void toBigEndianFrom64toByte(uint64_t src, uint8_t* des);
void toBigEndianFrom16toByte(uint16_t src, uint8_t* des);