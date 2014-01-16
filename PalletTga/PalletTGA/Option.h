// オプションパーサ

#ifndef __OPTION_H__
#define __OPTION_H__

#include <vector>


class Option
{
public:
	enum {
		OPTION_NAME_MAX_LENGTH = 256-1,
		OPTION_ARG_MAX_LENGTH = 256-1,
	};

	enum OPTION_INDEX {
		OPTION_INDEX_END = -1,
		OPTION_INDEX_INVALID_OPTION = -2,
		OPTION_INDEX_NOT_OPTION = 0,
		OPTION_INDEX_START = 1,
	};

	enum OPTION_ARG {
		OPTION_ARG_NEED = 0,
		OPTION_ARG_UNNEED,
		OPTION_ARG_INDIFFERENT,
	};

	enum OPTION_ERROR {
		OPTION_ERROR_SUCCESS = 0,
		OPTION_ERROR_INVALID_OPTION,
		OPTION_ERROR_NO_ARG,
		OPTION_ERROR_ORDER,
		OPTION_ERROR_NEED_OPTION,
		OPTION_ERROR_FAITAL,
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

public:
	Option(void);
	Option(int argc, char *argv[]);
	~Option(void);

	void SetArg(int argc, char *argv[]);
	void SetOption(int index, const char* name, OPTION_ARG is_arg, bool need);
	OPTION_ERROR CheckOption();
	int GetOption(char *name, char *arg);
	void SetBeginOption() { m_arg_index = 0; }
};


#endif // __OPTION_H__