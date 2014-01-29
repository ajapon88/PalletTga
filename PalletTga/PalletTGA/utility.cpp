#include "stdafx.h"
#include "utility.h"


bool IsHex(char hex)
{
	if ('0' <= hex && hex <= '9')  return true;
	if ('a' <= hex && hex <= 'f')  return true;
	if ('A' <= hex && hex <= 'F')  return true;

	return false;
}

bool IsHexStr(const char *hex)
{
	if (strncmp(hex, "0x", 2) == 0) {
		hex += 2;
	}
	while(*hex != '\0') {
		if (!IsHex(*hex)) {
			return false;
		}
		hex++;
	}

	return true;
}

int Hex2Int(char hex)
{
	if ('0' <= hex && hex <= '9')  return hex-'0';
	if ('a' <= hex && hex <= 'f')  return 10+hex-'a';
	if ('A' <= hex && hex <= 'F')  return 10+hex-'A';

	return 0;
}

int HexStr2Int(const char* hex)
{
	int res = 0;
	if (strncmp(hex, "0x", 2) == 0) {
		hex += 2;
	}
	while(*hex != '\0') {
		if (!IsHex(*hex)) {
			break;
		}
		res = (res<<4) + Hex2Int(*hex);
		hex++;
	}

	return res;
}

int Str2Int(const char *str)
{
	if (strncmp(str, "0x", 2) == 0) {
		return HexStr2Int(str);
	}
	return atoi(str);
}