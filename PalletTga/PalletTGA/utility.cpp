#include "utility.h"
#include <string.h>


bool IsHex(char hex)
{
	if ('0' <= hex && hex <= '9')  return true;
	if ('a' <= hex && hex <= 'f')  return true;
	if ('A' <= hex && hex <= 'F')  return true;

	return false;
}

int Hex2Int(char hex)
{
	if ('0' <= hex && hex <= '9')  return hex-'0';
	if ('a' <= hex && hex <= 'f')  return hex-'a';
	if ('A' <= hex && hex <= 'F')  return hex-'A';

	return 0;
}

int HexStr2Int(const char* hex)
{
	int res = 0;
	if (strncmp(hex, "0x", 2) == 0) {
		hex += 2;
	}
	while(hex != '\0') {
		if (!IsHex(*hex)) {
			break;
		}
		res = (res<<4) + Hex2Int(*hex);
		hex++;
	}

	return res;
}
