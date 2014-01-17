// �I�v�V�����p�[�T

#ifndef __OPTION_H__
#define __OPTION_H__

#include <vector>
#include "stdint.h"


class Option
{
public:
	// �I�v�V�����ԍ�
	enum OPTION_INDEX {
		OPTION_INDEX_END = -1,			// �I�v�V�����I��
		OPTION_INDEX_INVALID = -2,		// �����ȃI�v�V����
		OPTION_INDEX_NOT_OPTION = 0,	// �I�v�V��������Ȃ�
		OPTION_INDEX_START = 1,			// �I�v�V�����ԍ����߁D�I�v�V�����ԍ��͂���ȍ~���g��
	};

	// �I�v�V���������̗L��
	enum OPTION_ARG {
		OPTION_ARG_NEED = 0,	// �I�v�V���������K�{
		OPTION_ARG_UNNEED,		// �I�v�V���������s�v
		OPTION_ARG_INDIFFERENT,	// �I�v�V�������������Ă��Ȃ��Ă��悢
	};

	// �I�v�V�����`�F�b�N����
	enum OPTION_ERROR_FLAG {
		OPTION_ERROR_SUCCESS		= 0,	// �G���[�Ȃ�
		OPTION_ERROR_INVALID_OPTION	= 1<<0,	// �����ȃI�v�V����������
		OPTION_ERROR_NO_ARG			= 1<<1,	// �I�v�V���������K�{�ȃI�v�V�����ɃI�v�V��������������
		OPTION_ERROR_ORDER			= 1<<2, // �����̏��Ԃ����������B "�t�@�C���� [�I�v�V����] ���̑�" �̏�
		OPTION_ERROR_NEED_OPTION	= 1<<3,	// �K�{�I�v�V����������
	};

	// ���̑��萔
	enum {
		OPTION_NAME_MAX_LENGTH = 256-1,	// �I�v�V�������ő咷(NULL��������)
		OPTION_ARG_MAX_LENGTH = 256-1,	// �I�v�V���������ő咷(NULL��������)
	};

private:
	// �I�v�V�������
	typedef struct OptionInfo{
		int index;
		char name[OPTION_NAME_MAX_LENGTH+1];
		OPTION_ARG is_arg;
		bool need;
	};
	typedef std::vector<OptionInfo> OptionInfoList;

	int m_argc;			// �����̐�
	char **m_argv;		// ����
	int m_arg_index;	// �`�F�b�N���̈����ԍ�

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