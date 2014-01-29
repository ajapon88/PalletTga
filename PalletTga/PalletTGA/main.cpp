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
	
	enum {
		TGA_IMAGE_TYPE_RLE_FLAG = 8,
	};

	enum TGA_IMAGE_TYPE {
		TGA_IMAGE_TYPE_NONE = 0,
		TGA_IMAGE_TYPE_INDEX_COLOR,
		TGA_IMAGE_TYPE_FULL_COLOR,
		TGA_IMAGE_TYPE_MONOCHRO,
	};
}

void Usage(int argc, char *argv[]);
void InitTgaHeader(TgaHeader *header);
void InitTgaFooter(TgaFooter *footer);
void RLECompress(std::vector<uint8_t> *rle_data, const std::vector<uint8_t> &raw_data, int pixel_byte);


int main(int argc, char *argv[])
{
	Option option;
	Pallet pallet;
#if 0	// デバッグ用
	{
		char *_argv[] = {
			"",
			"-w", "100",
			"--height100",
			"-c", "0xEE00CC00",
//			"-p", "pallet.txt",
			"--RLE",
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
	option.SetOption(OPT_RLE, "--RLE", Option::OPTION_ARG_UNNEED, false);
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
	std::string option_name;
	std::string option_arg;
	while((opt = option.GetNextOption(&option_name, &option_arg)) != Option::OPTION_INDEX_END) {
#ifdef _DEBUG
		printf("opt: %d:%s(%s)\n", opt, option_name.c_str(), option_arg.c_str());
#endif
		switch (opt) {
			case OPT_W:
				width = atoi(option_arg.c_str());
				break;
			case OPT_H:
				height = atoi(option_arg.c_str());
				break;
			case OPT_COLOR:
				color = Str2Int(option_arg.c_str());
				break;
			case OPT_PALLET: {
					strncpy(palletfile, option_arg.c_str(), sizeof(palletfile));
					Pallet::PALLET_ERROR perror = pallet.LoadPalletFile(palletfile);
					if (Pallet::PALLET_SUCCESS != perror) {
						printf("Pallet error(%d): %s\n", perror, pallet.GetErrorMessage(perror));
						DEBUG_PAUSE();
						return -1;
					}
				}
				break;
			case OPT_RLE:
				is_rle = true;
				break;
			case Option::OPTION_INDEX_NOT_OPTION:
				if (strcmp(filename, "") == 0) {
					strncpy(filename, option_name.c_str(), sizeof(filename));
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
		header.image_type = TGA_IMAGE_TYPE_INDEX_COLOR;
		header.colormap_length = pallet.GetPalletCount();
		header.colormap_size = pallet.GetPalletSize();
	    header.bit_per_pixel = 0x08;
	} else {
		// パレットが無ければフルカラーにする
		header.image_type = TGA_IMAGE_TYPE_FULL_COLOR;
	    header.bit_per_pixel = 0x20;
	}
	if (is_rle) {
		header.image_type += TGA_IMAGE_TYPE_RLE_FLAG;
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
	std::vector<uint8_t> raw_data, rle_data, *output_data;
	for (int h = 0; h < header.height; h++) {
		for (int w = 0; w < header.width; w++) {
			if ((header.image_type&~TGA_IMAGE_TYPE_RLE_FLAG) == TGA_IMAGE_TYPE_INDEX_COLOR) {
				raw_data.push_back(static_cast<uint8_t>(color));
			} else if ((header.image_type&~TGA_IMAGE_TYPE_RLE_FLAG) == TGA_IMAGE_TYPE_FULL_COLOR) {
				// フルカラー
				uint8_t a = COLOR_ALPHA(color);
				uint8_t r = COLOR_RED(color);
				uint8_t g = COLOR_GREEN(color);
				uint8_t b = COLOR_BLUE(color);
				raw_data.push_back(b);
				raw_data.push_back(g);
				raw_data.push_back(r);
				raw_data.push_back(a);
			}
		}
	}
	if (header.image_type&TGA_IMAGE_TYPE_RLE_FLAG) {
		RLECompress(&rle_data, raw_data, header.bit_per_pixel/8);
		output_data = &rle_data;
	} else {
		output_data = &raw_data;
	}
	for (std::vector<uint8_t>::iterator it = output_data->begin(); it != output_data->end(); ++it) {
		char pixel = static_cast<char>(*it);
		ofs.write(&pixel, sizeof(pixel));
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
	printf("        -c color:  set pixel color\n");
	printf("        --RLE:  RLE commpress\n");
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

		// RLE圧縮（連続データのみ）
void RLECompress(std::vector<uint8_t> *rle_data, const std::vector<uint8_t> &raw_data, int pixel_byte)
{
	const uint8_t RLE_CONSECUTIVE_FLAG = 0x80;
	
	uint8_t *pixel = new uint8_t[pixel_byte];
	uint8_t *buf = new uint8_t[pixel_byte];
	uint8_t count = 0;
	int byte = 0;
	int state = 0;
	rle_data->clear();
	bool data_flag = false;
	for (std::vector<uint8_t>::const_iterator it = raw_data.begin(); it != raw_data.end(); ++it) {
		data_flag = true;
		switch (state) {
		// ピクセルデータ取得
		case 0:
			pixel[byte++] = *it;
			if (byte >= pixel_byte) {
				byte = 0;
				count = 0;
				state++;
			}
			break;
		// 連続数カウント
		case 1:
			buf[byte++] = *it;
			if (byte >= pixel_byte) {
				byte = 0;
				bool save = false;
				if (memcmp(pixel, buf, pixel_byte*sizeof(uint8_t)) == 0) {
					count++;
					if (count >= RLE_CONSECUTIVE_FLAG-1) {
						save = true;
						// 一旦フラグをおろす
						data_flag = false;
					}
				} else {
					save = true;
				}
				if (save) {
					rle_data->push_back(RLE_CONSECUTIVE_FLAG|count);
					for (int i = 0; i < pixel_byte; i++) {
						rle_data->push_back(pixel[i]);
					}
					memcpy(pixel, buf, pixel_byte*sizeof(uint8_t));
					count = 0;
				}
			}
			break;
		}
	}
	if (data_flag) {
		// あまったデータを追加
		rle_data->push_back(RLE_CONSECUTIVE_FLAG|count);
		for (int i = 0; i < pixel_byte; i++) {
			rle_data->push_back(pixel[i]);
		}
	}
	SAFE_DELETE_ARRAY(pixel);
	SAFE_DELETE_ARRAY(buf);
}
