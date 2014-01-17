#include "define.h"

#include "Option.h"
#include "Pallet.h"

#include <stdio.h>
#include <fstream>

#include "stdint.h"
//#include <stdint.h>

#include "utility.h"

#define COLOR_ARGB(a, r, g, b) (((a)<<24)+((r)<<16)+((g)<<8)+(b))
#define ALPHA(color) (((color)>>24)&0xFF)
#define RED(color) (((color)>>16)&0xFF)
#define GREEN(color) (((color)>>8)&0xFF)
#define BLUE(color) ((color)&0xFF)

#pragma pack(1)
// TGAヘッダ
typedef struct {
    uint8_t  id_field_length;
    uint8_t  colormap_type;
    uint8_t  image_type;
    uint16_t colormap_index;
    uint16_t colormap_length;
    uint8_t  colormap_size;
    uint16_t origin_x;
    uint16_t origin_y;
    uint16_t width;
    uint16_t height;
    uint8_t  bit_per_pixel;
    uint8_t  discripter;
} TgaHeader;

// TGAフッター
typedef struct {
    uint32_t file_position;
    uint32_t divelopper_directory;
    char     tga_sign[17];
    uint8_t  dummy;
} TgaFooter;

#pragma pack()

namespace {
	enum {
		OPT_W = Option::OPTION_INDEX_START,
		OPT_H,
		OPT_FILL,
		OPT_PALLET,
		OPT_RLE,
		OPT_HELP,
		OPT_MAX
	};
}

void Usage(int argc, char *argv[]);
void InitTgaHeader(TgaHeader *header);
void InitTgaFooter(TgaFooter *footer);


int main(int argc, char *argv[])
{
	Option option;
	Pallet pallet;
#if 0	// デバッグ用
	{
		char *_argv[] = {
			"実行ファイル名",
			"-w", "100",
			"-h100",
			"--fill", "0x10",
			"-p", "pallet.txt",
			"output.tga"
		};
		int _argc = ARRAY_SIZE(_argv);
		option.SetArg(_argc, _argv);
	}
#else
	option.SetArg(argc, argv);
#endif

	option.SetOption(OPT_W, "-w", Option::OPTION_ARG_NEED, true);
	option.SetOption(OPT_H, "-h", Option::OPTION_ARG_NEED, true);
	option.SetOption(OPT_FILL, "--fill", Option::OPTION_ARG_NEED, false);
	option.SetOption(OPT_PALLET, "-p", Option::OPTION_ARG_NEED, false);
	option.SetOption(OPT_HELP, "--help", Option::OPTION_ARG_UNNEED, false);

	if (option.GetOptionByIndex(OPT_HELP, NULL, NULL) != Option::OPTION_INDEX_INVALID) {
		Usage(argc, argv);
		DEBUG_PAUSE();
		return 0;
	}

	if (option.CheckOption() != Option::OPTION_ERROR_SUCCESS) {
		Usage(argc, argv);
		DEBUG_PAUSE();
		return -1;
	}

	uint16_t width, height;
	uint8_t fill = 0x00;
	bool is_rle = false;
	char filename[256] = "";

	int opt;
	char option_name[Option::OPTION_NAME_MAX_LENGTH+1];
	char option_arg[Option::OPTION_ARG_MAX_LENGTH+1];
	while((opt = option.GetNextOption(option_name, option_arg)) != Option::OPTION_INDEX_END) {
#ifdef _DEBUG
		printf("opt: %d:%s(%s)\n", opt, option_name, option_arg);
#endif
		switch (opt) {
			case OPT_W:
				width = atoi(option_arg);
				break;
			case OPT_H:
				height = atoi(option_arg);
				break;
			case OPT_FILL:
				if (strncmp(option_arg, "0x", 2) == 0) {
					fill = HexStr2Int(option_arg);
				} else {
					fill = atoi(option_arg);
				}
				break;
			case OPT_PALLET: {
					Pallet::PALLET_ERROR perror = pallet.LoadPalletFile(option_arg);
					if (Pallet::PALLET_SUCCESS != perror) {
						printf("Pallet error(%d)\n", perror);
						DEBUG_PAUSE();
						return -1;
					}
				}
				break;
			case Option::OPTION_INDEX_NOT_OPTION:
				if (strcmp(filename, "") == 0) {
					strncpy(filename, option_name, sizeof(filename));
				}
				break;
		}
	}

	if (strcmp(filename, "") == 0) {
		Usage(argc, argv);
		DEBUG_PAUSE();
		return -1;
	}

	printf("output: %s\n", filename);
	printf("width: %d\n", width);
	printf("height: %d\n", height);
	printf("fill pixel: 0x%02x\n", fill);

	TgaHeader header;
	TgaFooter footer;
	
	InitTgaHeader(&header);
	header.width = width;
	header.height = height;
	if (pallet.GetPalletCount() > 0) {
		header.colormap_type = 0x01;
		header.image_type = 0x01;
		header.colormap_length = pallet.GetPalletCount();
		header.colormap_size = pallet.GetPalletSize();
	    header.bit_per_pixel = 0x08;
	} else {
		// パレットが無ければフルカラーにする
		header.image_type = 0x02;
	    header.bit_per_pixel = 0x20;
	}

	InitTgaFooter(&footer);

	std::ofstream ofs(filename, std::ios::binary);
	if (ofs.fail()) {
		printf("file open failed. filename=%s\n", filename);
		DEBUG_PAUSE();
		return -1;
	}

	// ヘッダ
	ofs.write((char*)&header, sizeof(TgaHeader));
	// パレット
	for (int i = 0; i < pallet.GetPalletCount(); i++) {
		uint8_t a = pallet.GetPalletColorA(i);
		uint8_t r = pallet.GetPalletColorR(i);
		uint8_t g = pallet.GetPalletColorG(i);
		uint8_t b = pallet.GetPalletColorB(i);

		if (pallet.GetPalletSize() == Pallet::PALLET_SIZE_RGB) {
			ofs.write((char*)&b, sizeof(b));
			ofs.write((char*)&g, sizeof(g));
			ofs.write((char*)&r, sizeof(r));
		} else {
			ofs.write((char*)&b, sizeof(b));
			ofs.write((char*)&g, sizeof(g));
			ofs.write((char*)&r, sizeof(r));
			ofs.write((char*)&a, sizeof(a));
		}
	}
	// データ
	for (int h = 0; h < header.height; h++) {
		for (int w = 0; w < header.width; w++) {
			if (header.image_type == 0x01) {
				ofs.write((char*)&fill, sizeof(fill));
			} else {
				// フルカラー
				ofs.write((char*)&fill, sizeof(fill));
				ofs.write((char*)&fill, sizeof(fill));
				ofs.write((char*)&fill, sizeof(fill));
				ofs.write((char*)&fill, sizeof(fill));
			}
		}
	}
	// フッタ
	ofs.write((char*)&footer, sizeof(TgaFooter));

	ofs.close();

	DEBUG_PAUSE();
	return 0;
}


// コマンド説明
void Usage(int argc, char *argv[])
{
	printf("Usage: %s -w width -h height [options] output\n", argv[0]);
	printf("        --help:  show usage\n");
	printf("        --fill fill_pixel:  set fill pixel\n");
}

// TGAヘッダ初期化
void InitTgaHeader(TgaHeader *header)
{
    memset(header, 0, sizeof(TgaHeader));
    header->id_field_length = 0x00;
    header->colormap_type = 0x00;
    header->image_type = 0x00;
    header->colormap_index = 0x0000;
    header->colormap_length = 0x0000;
    header->colormap_size = 0x00;
    header->origin_x = 0x0000;
    header->origin_y = 0x0000;
    header->width = 0x0000;
    header->height = 0x0000;
    header->bit_per_pixel = 0x08;
    header->discripter = 0x00;
}

// TGAフッタ初期化
void InitTgaFooter(TgaFooter *footer)
{
    memset(footer, 0, sizeof(TgaFooter));
    footer->file_position =0x00000000;
    footer->divelopper_directory = 0x00000000;
    memcpy(footer->tga_sign, "TRUEVISION-TARGA", std::min(sizeof(footer->tga_sign), strlen("TRUEVISION-TARGA")+1));
    footer->dummy = 0x00;
}
