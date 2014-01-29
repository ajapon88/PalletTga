#ifndef __UTILITY_H__
#define __UTILITY_H__
#include "stdafx.h"

bool IsHex(char hex);
bool IsHexStr(const char *hex);
int Hex2Int(char hex);
int HexStr2Int(const char* hex);
int Str2Int(const char *str);

#endif // __UTILITY_H__