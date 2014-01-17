// オプションパーサ

#ifndef __OPTION_H__
#define __OPTION_H__

#include <vector>
#include "stdint.h"


class Option
{
public:
	enum {
		OPTION_NAME_MAX_LENGTH = 256-1,
		OPTION_ARG_MAX_LENGTH = 256-1,
	};

	enum OPTION_INDEX {
		OPTION_INDEX_END = -1,
		OPTION_INDEX_INVALID = -2,
		OPTION_INDEX_NOT_OPTION = 0,
		OPTION_INDEX_START = 1,
	};

	enum OPTION_ARG {
		OPTION_ARG_NEED = 0,
		OPTION_ARG_UNNEED,
		OPTION_ARG_INDIFFERENT,
	};

	enum OPTION_ERROR_FLAG {
		OPTION_ERROR_SUCCESS		= 0,
		OPTION_ERROR_INVALID_OPTION	= 1<<0,
		OPTION_ERROR_NO_ARG			= 1<<1,
		OPTION_ERROR_ORDER			= 1<<2,
		OPTION_ERROR_NEED_OPTION	= 1<<3,
	};

private:
	typedef struct OptionInfo{
		int index;
		char name[OPTION_NAME_MAX_LENGTH+1];
		OPTION_ARG is_arg;
		bool need;
	};
	typedef std::vector<OptionInfo> OptionInfoList;

	int m_argc;
	char **m_argv;
	int m_arg_index;

	OptionInfoList m_optinfo;


	void DestroyArg();
	int CheckOptionByArgIndex(unsigned int arg_index, const OptionInfo *optinfo, char *option_arg);

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