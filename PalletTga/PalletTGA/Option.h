// オプションパーサ
#ifndef __OPTION_H__
#define __OPTION_H__
#include "stdafx.h"


class Option
{
public:
	// オプション番号
	enum OPTION_INDEX {
		OPTION_INDEX_END = -1,			// オプション終了
		OPTION_INDEX_INVALID = -2,		// 無効なオプション
		OPTION_INDEX_NOT_OPTION = 0,	// オプションじゃない
		OPTION_INDEX_START = 1,			// オプション番号初め．オプション番号はこれ以降を使う
	};

	// オプション引数の有無
	enum OPTION_ARG {
		OPTION_ARG_NEED = 0,	// オプション引数必須
		OPTION_ARG_UNNEED,		// オプション引数不要
		OPTION_ARG_INDIFFERENT,	// オプション引数あってもなくてもよい
	};

	// オプションチェック結果
	enum OPTION_ERROR_FLAG {
		OPTION_ERROR_SUCCESS		= 0,	// エラーなし
		OPTION_ERROR_INVALID_OPTION	= 1<<0,	// 無効なオプションがある
		OPTION_ERROR_NO_ARG			= 1<<1,	// オプション引数必須なオプションにオプション引数が無い
		OPTION_ERROR_ORDER			= 1<<2, // 引数の順番がおかしい。 "ファイル名 [オプション] その他" の順
		OPTION_ERROR_NEED_OPTION	= 1<<3,	// 必須オプションが無い
	};

	// その他定数
	enum {
		OPTION_NAME_MAX_LENGTH = 256-1,	// オプション名最大長(NULL文字除く)
		OPTION_ARG_MAX_LENGTH = 256-1,	// オプション引数最大長(NULL文字除く)
	};

private:
	// オプション情報
	typedef struct OptionInfo{
		char name[OPTION_NAME_MAX_LENGTH+1];
		OPTION_ARG is_arg;
		bool need;
	};
	typedef std::map<int, OptionInfo> OptionInfoList;

	int m_argc;			// 引数の数
	char **m_argv;		// 引数
	int m_arg_index;	// チェック中の引数番号

	OptionInfoList m_optinfo;


	void DestroyArg();
	int CheckOptionByArgIndex(unsigned int arg_index, const OptionInfo &optinfo, char *option_arg);

public:
	Option(void);
	Option(int argc, char *argv[]);
	~Option(void);

	void SetArg(int argc, char *argv[]);
	void SetOption(int index, const char* name, OPTION_ARG is_arg, bool need);
	uint32_t CheckOption();
	int GetNextOption(char *name, char *arg);
	int GetOptionByIndex(int option_index, char *name, char *arg);
	void ResetOptionIndex() { m_arg_index = 0; }
};


#endif // __OPTION_H__