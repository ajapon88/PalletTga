#include "stdafx.h"
#include "Option.h"


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
	OptionInfo info;

	memset(&info, 0, sizeof(OptionInfo));
	strncpy(info.name, name, OPTION_NAME_MAX_LENGTH);
	info.is_arg = is_arg;
	info.need = need;

	m_optinfo.insert(OptionInfoList::value_type(index, info));
}

// �I�v�V�����ȈՃ`�F�b�N
// �����ȃI�v�V�����A�I�v�V�����̏��ԁA�I�v�V�������������邩�ǂ����A�K�{�I�v�V���������邩�ǂ������`�F�b�N
uint32_t Option::CheckOption()
{
	uint32_t error = OPTION_ERROR_SUCCESS;
	int old_arg_index = m_arg_index;
	m_arg_index = 0;

	bool not_option = false;
	std::set<int> exists;
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

		exists.insert(opt);

		// �p�����[�^�`�F�b�N
		OptionInfoList::iterator it = m_optinfo.find(opt);
		if (it != m_optinfo.end()) {
			if (it->second.is_arg == OPTION_ARG_NEED && strcmp(arg, "") == 0) {
				error |= OPTION_ERROR_NO_ARG;
			}
		}
	}

	// �K�{�I�v�V�����`�F�b�N
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

// �������I�v�V�����ƈ�v���邩�ǂ����`�F�b�N�B�������擾����
// -1: �s��v
//  0�ȏ�: ��v�B�Ԃ�l�̓C���f�b�N�X��i�߂鐔
int Option::CheckOptionByArgIndex(unsigned int arg_index, const OptionInfo &optinfo, char *optarg)
{
	int ret = -1;
	bool match = false;
	char optname[OPTION_ARG_MAX_LENGTH+1];

	if (optarg) optarg[0] = '\0';
	memcpy(optname, optinfo.name, sizeof(optname));

	char *p = optname;
	char *name = NULL;

	while(name = strtok(p, "|")) {
		p = NULL;

		if (strcmp(m_argv[arg_index], name) == 0) {
			match = true;
			ret = 0;
		}

		if (optinfo.is_arg == OPTION_ARG_NEED || optinfo.is_arg == OPTION_ARG_INDIFFERENT) {
			if (match) {
				if (optarg && arg_index+1 < m_argc) {
					if (m_argv[arg_index+1][0] != '-') {
						strncpy(optarg, m_argv[arg_index+1], OPTION_ARG_MAX_LENGTH+1);
						ret = 1;
					}
				}
			} else {
				int len = strlen(name);
				if (strncmp(m_argv[arg_index], name, len) == 0) {
					match = true;
					ret = 0;
					if (optarg) {
						strncpy(optarg, m_argv[arg_index] + len, OPTION_ARG_MAX_LENGTH+1);
					}
				}
			}
		}

		if (match) {
			return ret;
		}
	}

	return -1;
}

// �I�v�V�����擾
// �Ăяo���Ǝ����Ŏ��̃I�v�V�����֍s��
// �I�v�V�����ԍ����Ԃ�
int Option::GetNextOption(char *name, char *arg)
{
	if (name) name[0] = '\0';
	if (arg) arg[0] = '\0';

	if (m_arg_index >= m_argc) {
		return OPTION_INDEX_END;
	}

	for (OptionInfoList::iterator it = m_optinfo.begin(); it != m_optinfo.end(); ++it) {
		int shift = CheckOptionByArgIndex(m_arg_index, it->second, arg);

		if (shift >= 0) {
			if (name){
				strcpy(name, m_argv[m_arg_index]);
			}
			m_arg_index += shift+1;
			return it->first;
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
// �w�肵���ԍ��̃I�v�V�������擾����
// ���݂��Ȃ���� OPTION_INDEX_INVALID ���Ԃ�
int Option::GetOptionByIndex(int option_index, char *name, char *arg)
{
	if (name) name[0] = '\0';
	if (arg) arg[0] = '\0';

	OptionInfoList::iterator it = m_optinfo.find(option_index);
	if (it != m_optinfo.end()) {
		for (int i = 0; i < m_argc; i++) {
			int shift = CheckOptionByArgIndex(i, it->second, arg);
			if (shift >= 0) {
				if (name){
					strcpy(name, m_argv[i]);
				}
				return it->first;
			}
		}
	}

	return OPTION_INDEX_INVALID;
}
