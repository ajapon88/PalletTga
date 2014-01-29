#include "stdafx.h"
#include "Option.h"


Option::Option(void)
: m_argc(0)
, m_arg_index(0)
{
}

Option::Option(int argc, char *argv[])
: m_argc(0)
, m_arg_index(0)
{
	SetArg(argc, argv);
}

Option::~Option(void)
{
	DestroyArg();
	m_optinfo.clear();
}

// オプション引数破棄
void Option::DestroyArg()
{
	m_argv.clear();
	m_argc = 0;
	m_arg_index = 0;
}

// オプション引数セット
void Option::SetArg(int argc, char *argv[])
{
	DestroyArg();

	m_argc = argc-1;
	for (int i = 0; i < m_argc; i++) {
		std::string arg(argv[i+1]);
		m_argv.push_back(arg);
	}
	m_arg_index = 0;
}

// オプション設定
void Option::SetOption(int index, const char* name, OPTION_ARG is_arg, bool need)
{
	OptionInfo info;

	memset(&info, 0, sizeof(OptionInfo));
	info.index = index;
	info.is_arg = is_arg;
	info.need = need;

	m_optinfo.insert(OptionInfoList::value_type(index, info));

	// オプションネームを|区切りで分けて格納する
	int len = strlen(name);
	char *buf = new char[len+1];
	strncpy(buf, name, len+1);
	char *p = buf;
	char *t;
	while(t = strtok(p, "|")) {
		p = NULL;
		std::string optname(t);
		OptionNameList::iterator it = m_optname.find(index);
		if (it == m_optname.end()) {
			std::set<std::string> namelist;
			namelist.insert(optname);
			m_optname.insert(OptionNameList::value_type(index, namelist));
		} else {
			it->second.insert(optname);
		}
	}

#if 0
	{
		printf("OPTNAMES\n");
		for (OptionNameList::iterator namelist_it = m_optname.begin(); namelist_it != m_optname.end(); ++namelist_it) {
			printf("  index:%d\n", namelist_it->first);
			for (std::set<std::string>::reverse_iterator name_it = namelist_it->second.rbegin(); name_it != namelist_it->second.rend(); ++name_it) {
				printf("    name:%s\n", name_it->c_str());
			}
		}
	}
#endif
}

// オプション簡易チェック
// 無効なオプション、オプションの順番、オプション引数があるかどうか、必須オプションがあるかどうかをチェック
uint32_t Option::CheckOption()
{
	uint32_t error = OPTION_ERROR_SUCCESS;
	int old_arg_index = m_arg_index;
	m_arg_index = 0;

	bool not_option = false;
	std::set<int> exists;
	std::string name;
	std::string arg;
	int opt;
	while((opt = GetNextOption(&name, &arg)) != OPTION_INDEX_END) {
		if (OPTION_INDEX_INVALID == opt) {
			// 未定義のオプション
			error |= OPTION_ERROR_INVALID_OPTION;
			continue;
		}
		if (OPTION_INDEX_NOT_OPTION == opt) {
			not_option = true;
			continue;
		}
		if (not_option) {
			// オプションが先に来ていない
			error |= OPTION_ERROR_ORDER;
			continue;
		}

		exists.insert(opt);

		// パラメータチェック
		OptionInfoList::iterator it = m_optinfo.find(opt);
		if (it != m_optinfo.end()) {
			if (it->second.is_arg == OPTION_ARG_NEED && arg.length() == 0) {
				error |= OPTION_ERROR_NO_ARG;
			}
		}
	}

	// 必須オプションチェック
	if (error == OPTION_ERROR_SUCCESS) {
		for (OptionInfoList::iterator list_it = m_optinfo.begin(); list_it != m_optinfo.end(); ++list_it) {
			if (list_it->second.need) {
				std::set<int>::iterator exists_it = exists.find(list_it->first);
				if (exists_it == exists.end()) {
					error |= OPTION_ERROR_NEED_OPTION;
					break;
				}
			}
		}
	}

	m_arg_index = old_arg_index;

	return error;
}

// 引数がオプションと一致するかどうかチェック。引数も取得する
// -1: 不一致
//  0以上: 一致。返り値はインデックスを進める数
int Option::CheckOptionByArgIndex(unsigned int arg_index, const OptionInfo &optinfo, std::string *optarg)
{
	int ret = -1;
	bool match = false;

	if (optarg) optarg->clear();
	

	OptionNameList::iterator optname_it = m_optname.find(optinfo.index);
	if (optname_it != m_optname.end()) {
		// 辞書式順に並んでいるので逆順にオプション名が長いものからチェックする
		for (std::set<std::string>::reverse_iterator name_it = optname_it->second.rbegin(); name_it != optname_it->second.rend(); ++name_it) {

			if (m_argv[arg_index].compare(*name_it) == 0) {
				match = true;
				ret = 0;
			}

			if (optinfo.is_arg == OPTION_ARG_NEED || optinfo.is_arg == OPTION_ARG_INDIFFERENT) {
				if (match) {
					if (arg_index+1 < static_cast<unsigned int>(m_argc)) {
						if (m_argv[arg_index+1].compare(0, 1, "-") != 0) {
							if (optarg) {
								*optarg = m_argv[arg_index+1];
							}
							ret = 1;
						}
					}
				} else {
					int len = name_it->length();
					if (m_argv[arg_index].compare(0, len, *name_it) == 0) {
						match = true;
						ret = 0;
						if (optarg) {
							*optarg = m_argv[arg_index].substr(len);
						}
					}
				}
			}

			if (match) {
				return ret;
			}
		}
	}

	return -1;
}

// オプション取得
// 呼び出すと自動で次のオプションへ行く
// オプション番号が返る
int Option::GetNextOption(std::string *name, std::string *arg)
{
	if (name) name->clear();
	if (arg) arg->clear();

	if (m_arg_index >= m_argc) {
		return OPTION_INDEX_END;
	}

	for (OptionInfoList::iterator it = m_optinfo.begin(); it != m_optinfo.end(); ++it) {
		int shift = CheckOptionByArgIndex(m_arg_index, it->second, arg);

		if (shift >= 0) {
			if (name) {
				*name = m_argv[m_arg_index];
			}
			m_arg_index += shift+1;
			return it->first;
		}
	}

	if (name) {
		*name = m_argv[m_arg_index];
	}
	m_arg_index++;

	if (name && name->compare(0, 1, "-") == 0) {
		return OPTION_INDEX_INVALID;
	}
	return OPTION_INDEX_NOT_OPTION;
}


// オプション取得
// 指定した番号のオプションを取得する
// 存在しなければ OPTION_INDEX_INVALID が返る
int Option::GetOptionByIndex(int option_index, std::string *name, std::string *arg)
{
	if (name) name->clear();
	if (arg) arg->clear();

	OptionInfoList::iterator it = m_optinfo.find(option_index);
	if (it != m_optinfo.end()) {
		for (int i = 0; i < m_argc; i++) {
			int shift = CheckOptionByArgIndex(i, it->second, arg);
			if (shift >= 0) {
				if (name) {
					*name = m_argv[i];
				}
				return it->first;
			}
		}
	}

	return OPTION_INDEX_INVALID;
}
