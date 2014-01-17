#ifndef __PALLET_H__
#define __PALLET_H__
#include "stdint.h"
#include "color.h"

class Pallet
{
public:
	enum PALLET_SIZE {
		PALLET_SIZE_INVALID = -1,
		PALLET_SIZE_RGB = 24,
		PALLET_SIZE_ARGB = 32,
	};

	enum PALLET_BYTE_SIZE {
		PALLET_BYTE_SIZE_RGB = PALLET_SIZE_RGB / 4,
		PALLET_BYTE_SIZE_ARGB = PALLET_SIZE_ARGB / 4,
	};

	enum PALLET_ERROR {
		PALLET_SUCCESS = 0,
		PALLET_ERROR_FILE_OPEN_FAILED,
		PALLET_ERROR_NOT_HEX_FORMAT,
		PALLET_ERROR_NOT_COLOR_FORMAT,
		PALLET_ERROR_SIZE_NOT_MATCH,
	};

	enum {
		PALLET_MAX = 256,
	};

	Pallet(void);
	~Pallet(void);

	PALLET_ERROR LoadPalletFile(const char* filename);

	void SetPalletSize(PALLET_SIZE size) { m_pallet_size = size; }
	PALLET_SIZE GetPalletSize() const { return m_pallet_size; }
	int GetPalletCount() const { return m_pallet_count; }
	void AddPalletColor(uint8_t a, uint8_t r, uint8_t g, uint8_t b) { m_color[m_pallet_count++].SetColor(r, g, b, a); }
	void AddPalletColor(uint8_t r, uint8_t g, uint8_t b) { m_color[m_pallet_count++].SetColor(r, g, b); }
	void AddPalletColor(const Color &color) { m_color[m_pallet_count++].SetColor(color); }
	Color &GetPalletColor(int index) { return m_color[index]; }
	uint8_t GetPalletColorA(int index) const { return m_color[index].GetA(); }
	uint8_t GetPalletColorR(int index) const { return m_color[index].GetR(); }
	uint8_t GetPalletColorG(int index) const { return m_color[index].GetG(); }
	uint8_t GetPalletColorB(int index) const { return m_color[index].GetB(); }

private:
	PALLET_SIZE m_pallet_size;
	int m_pallet_count;
	Color m_color[PALLET_MAX];
};

#endif // __PALLET_H__