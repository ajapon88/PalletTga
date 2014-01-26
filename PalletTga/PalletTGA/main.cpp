#include "stdafx.h"
#include "Option.h"
#include "Pallet.h"
#include "utility.h"


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
		OPT_COLOR,
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
			"--height100",
			"-c", "0x10",
			"-p", "pallet.txt",
			"output.tga"
		};
		int _argc = ARRAY_SIZE(_argv);
		option.SetArg(_argc, _argv);
	}
#else
	option.SetArg(argc, argv);
#endif

	option.SetOption(OPT_W, "-w|--width", Option::OPTION_ARG_NEED, true);
	option.SetOption(OPT_H, "-h|--height", Option::OPTION_ARG_NEED, true);
	option.SetOption(OPT_COLOR, "-c|--color", Option::OPTION_ARG_NEED, false);
	option.SetOption(OPT_PALLET, "-p|--pallet", Option::OPTION_ARG_NEED, false);
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
	uint32_t color = 0;
	bool is_rle = false;
	char filename[256] = "";
	char palletfile[256] = "";

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
			case OPT_COLOR:
				if (strncmp(option_arg, "0x", 2) == 0) {
					color = HexStr2Int(option_arg);
				} else {
					color = atoi(option_arg);
				}
				break;
			case OPT_PALLET: {
					strncpy(palletfile, option_arg, sizeof(palletfile));
					Pallet::PALLET_ERROR perror = pallet.LoadPalletFile(palletfile);
					if (Pallet::PALLET_SUCCESS != perror) {
						printf("Pallet error(%d): %s\n", perror, pallet.GetErrorMessage(perror));
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
	if (strcmp(palletfile, "") != 0) {
		printf("pallet: %s\n", palletfile);
	}
	printf("color: 0x%02x\n", color);

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
				uint8_t pixel = static_cast<uint8_t>(color);
				ofs.write((char*)&pixel, sizeof(pixel));
			} else {
				// フルカラー
				uint8_t a = COLOR_ALPHA(color);
				uint8_t r = COLOR_RED(color);
				uint8_t g = COLOR_GREEN(color);
				uint8_t b = COLOR_BLUE(color);
				ofs.write((char*)&b, sizeof(b));
				ofs.write((char*)&g, sizeof(g));
				ofs.write((char*)&r, sizeof(r));
				ofs.write((char*)&a, sizeof(a));
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
	printf("        --c color:  set pixel color\n");
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
