#include "stdafx.h"
#include "Pallet.h"
#include "utility.h"


Pallet::Pallet(void)
: m_pallet_size(PALLET_SIZE_INVALID)
, m_pallet_count(0)
{
}

Pallet::~Pallet(void)
{
}

// パレットファイル読み込み
// RGB、もしくはARGB形式で空白区切りのデータを読める
Pallet::PALLET_ERROR Pallet::LoadPalletFile(const char* filename)
{
	m_pallet_count = 0;
	m_pallet_size = PALLET_SIZE_INVALID;

	std::ifstream ifs(filename);
	if (ifs.fail()) {
		return PALLET_ERROR_FILE_OPEN_FAILED;
	}

	std::string col;
	int pallet_size = -1;
	while(!ifs.eof()) {
		ifs >> col;
		if (col.length() == 0) {
			continue;
		}
		if (!IsHexStr(col.c_str())) {
			return PALLET_ERROR_NOT_HEX_FORMAT;
		}
		int len = col.length();
		int size;
		if (len == PALLET_BYTE_SIZE_RGB) {
			size = PALLET_SIZE_RGB;
		} else if (len == PALLET_BYTE_SIZE_ARGB) {
			size = PALLET_SIZE_ARGB;
		} else {
			return PALLET_ERROR_NOT_COLOR_FORMAT;
		}
		if (pallet_size == -1) {
			pallet_size = size;
		} else if (size != pallet_size) {
			return PALLET_ERROR_SIZE_NOT_MATCH;
		}
		uint32_t color = HexStr2Int(col.c_str());
		if (pallet_size == PALLET_BYTE_SIZE_RGB) {
			AddPalletColor((color>>16)&0xFF, (color>>8)&0xFF, color&0xFF);
		} else {
			AddPalletColor((color>>24)&0xFF, (color>>16)&0xFF, (color>>8)&0xFF, color&0xFF);
		}
	}
	ifs.close();
	m_pallet_size = (PALLET_SIZE)pallet_size;

	return PALLET_SUCCESS;
}


const char *Pallet::GetErrorMessage(PALLET_ERROR errono)
{
	switch (errono) {
	case PALLET_ERROR_FILE_OPEN_FAILED:
		return "Pallet file open failed.";
	case PALLET_ERROR_NOT_HEX_FORMAT:
		return "Pallet color format is not hex.";
	case PALLET_ERROR_NOT_COLOR_FORMAT:
		return "Pallet color is invalid format .";
	case PALLET_ERROR_SIZE_NOT_MATCH:
		return "Pallet color size is not match.";
	}
	return "Fatal error.";
}