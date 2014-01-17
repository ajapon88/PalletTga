
#include "Option.h"

#include "define.h"


Option::Option(void)
: m_argc(0)
, m_argv()
, m_arg_index(0)
{
}

Option::Option(int argc, char *argv[])
: m_argc(0)
, m_argv()
, m_arg_index(0)
{
	SetArg(argc, argv);
}

Option::~Option(void)
{
	DestroyArg();
	m_optinfo.clear();
}

// �I�v�V���������j��
void Option::DestroyArg()
{
	for (int i = 0; i < m_argc; i++) {
		SAFE_DELETE_ARRAY(m_argv[i]);
	}
	SAFE_DELETE(m_argv);
	m_argc = 0;
	m_arg_index = 0;
}

// �I�v�V���������Z�b�g
void Option::SetArg(int argc, char *argv[])
{
	DestroyArg();

	m_argc = argc-1;
	m_argv = new char*[m_argc];
	for (int i = 0; i < m_argc; i++) {
		int size = strlen(argv[i+1]) + 1;
		m_argv[i] = new char[size];
		strncpy(m_argv[i], argv[i+1], size);
	}
	m_arg_index = 0;
}

// �I�v�V�����ݒ�
void Option::SetOption(int index, const char* name, OPTION_ARG is_arg, bool need)
{
	for (OptionInfoList::iterator it = m_optinfo.begin(); it != m_optinfo.end(); ++it) {
		if (it->index == index) {
			strncpy(it->name, name, OPTION_NAME_MAX_LENGTH);
			it->is_arg = is_arg;
			it->need = need;

			return;
		}
	}

	OptionInfo info;

	memset(&info, 0, sizeof(OptionInfo));
	info.index = index;
	strncpy(info.name, name, OPTION_NAME_MAX_LENGTH);
	info.is_arg = is_arg;
	info.need = need;

	m_optinfo.push_back(info);
}

// �I�v�V�����ȈՃ`�F�b�N
uint32_t Option::CheckOption()
{
	uint32_t error = OPTION_ERROR_SUCCESS;
	int old_arg_index = m_arg_index;
	m_arg_index = 0;

	bool not_option = false;
	std::vector<int> exists;
	char name[OPTION_NAME_MAX_LENGTH+1];
	char arg[OPTION_ARG_MAX_LENGTH+1];
	int opt;
	while((opt = GetNextOption(name, arg)) != OPTION_INDEX_END) {
		if (OPTION_INDEX_INVALID == opt) {
			// ����`�̃I�v�V����
			error |= OPTION_ERROR_INVALID_OPTION;
			continue;
		}
		if (OPTION_INDEX_NOT_OPTION == opt) {
			not_option = true;
			continue;
		}
		if (not_option) {
			// �I�v�V��������ɗ��Ă��Ȃ�
			error |= OPTION_ERROR_ORDER;
			continue;
		}

		exists.push_back(opt);

		// �p�����[�^�`�F�b�N
		for (OptionInfoList::iterator it = m_optinfo.begin(); it != m_optinfo.end(); ++it) {
			if (it->index == opt) {
				if (it->is_arg == OPTION_ARG_NEED && strcmp(arg, "") == 0) {
					error |= OPTION_ERROR_NO_ARG;
				}

				break;
			}
		}
	}

	// �K�{�I�v�V�����`�F�b�N
	if (error == OPTION_ERROR_SUCCESS) {
		for (OptionInfoList::iterator list_it = m_optinfo.begin(); list_it != m_optinfo.end(); ++list_it) {
			if (list_it->need) {
				bool e = false;
				for (std::vector<int>::iterator exists_it = exists.begin(); exists_it != exists.end(); ++exists_it) {
					if (list_it->index == *exists_it) {
						e = true;
						break;
					}
				}
				if (!e) {
					error |= OPTION_ERROR_NEED_OPTION;
					break;
				}
			}
		}
	}

	m_arg_index = old_arg_index;

	return error;
}

// �������I�v�V�����ƈ�v���邩�ǂ����`�F�b�N�B�������擾����
// -1: �s��v
//  0�ȏ�: ��v�B�Ԃ�l�̓C���f�b�N�X��i�߂鐔
int Option::CheckOptionByArgIndex(unsigned int arg_index, const OptionInfo *optinfo, char *optarg)
{
	int ret = -1;
	bool match = false;

	if (optarg) optarg[0] = '\0';

	if (strcmp(m_argv[arg_index], optinfo->name) == 0) {
		match = true;
		ret = 0;
	}

	if (optinfo->is_arg == OPTION_ARG_NEED || optinfo->is_arg == OPTION_ARG_INDIFFERENT) {
		if (match) {
			if (optarg && arg_index+1 < m_argc) {
				if (m_argv[arg_index+1][0] != '-') {
					strncpy(optarg, m_argv[arg_index+1], OPTION_ARG_MAX_LENGTH+1);
					ret = 1;
				}
			}
		} else {
			int len = strlen(optinfo->name);
			if (strncmp(m_argv[arg_index], optinfo->name, len) == 0) {
				ret = 0;
				if (optarg) {
					strncpy(optarg, m_argv[arg_index] + len, OPTION_ARG_MAX_LENGTH+1);
				}
			}
		}
	}

	return ret;
}

// �I�v�V�����擾
int Option::GetNextOption(char *name, char *arg)
{
	if (name) name[0] = '\0';
	if (arg) arg[0] = '\0';

	if (m_arg_index >= m_argc) {
		return OPTION_INDEX_END;
	}

	for (OptionInfoList::iterator it = m_optinfo.begin(); it != m_optinfo.end(); ++it) {
		int shift = CheckOptionByArgIndex(m_arg_index, &(*it), arg);

		if (shift >= 0) {
			m_arg_index += shift+1;
			if (name){
				strcpy(name, it->name);
			}
			return it->index;
		}
	}

	strncpy(name, m_argv[m_arg_index], OPTION_NAME_MAX_LENGTH+1);
	m_arg_index++;

	if (name[0] == '-') {
		return OPTION_INDEX_INVALID;
	}
	return OPTION_INDEX_NOT_OPTION;
}


// �I�v�V�����擾
int Option::GetOptionByIndex(int option_index, char *name, char *arg)
{
	if (name) name[0] = '\0';
	if (arg) arg[0] = '\0';

	for (OptionInfoList::iterator it = m_optinfo.begin(); it != m_optinfo.end(); ++it) {
		if (it->index == option_index) {
			for (int i = 0; i < m_argc; i++) {
				int shift = CheckOptionByArgIndex(i, &(*it), arg);
				if (shift >= 0) {
					if (name) {
						strcpy(name, it->name);
					}
					return it->index;
				}
			}

			break;
		}
	}

	return OPTION_INDEX_INVALID;
}
