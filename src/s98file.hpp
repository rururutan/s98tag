#pragma once
//////////////////////////////////////////////////////////
//
//
//
// $Date: 2005/08/05 02:54:25 $ / $Revision: 1.1 $
//
//////////////////////////////////////////////////////////

#include <list>
#include "s98v3tag.hpp"

class s98file {

  public:

	s98file();
	~s98file();

	// file
	bool read(char *path);
	bool update();

	// tag
	bool is_unicode();
	int get_tag_count();
	bool set_code(s98v3tag::TAG_CODE);
	s98v3tag* get_tag(int count);
	s98v3tag* get_tag(char *tag_name);
	bool set_tag(char *tag_name, char *value);
	bool add_tag(char *tag_name, char *value);
	bool remove_tag(char *tag_name);

	// info
	int get_version();

	// convert
	bool convert_version(unsigned int version);

  private:

	// �t�@�C���p�X
	char *m_pPath;
	// �ǂݍ��񂾃^�O�̃������C���[�W
	char *m_pTag;
	// �ǂݍ��񂾃^�O�̃������C���[�W�T�C�Y
	int m_tag_size;
	// �^�O�̕����R�[�h
	s98v3tag::TAG_CODE m_tag_code;
	// �^�O���
	std::list<s98v3tag*> m_pTagList;
	// �^�O�J�n�I�t�Z�b�g
	int m_tag_offset;
	// S98�o�[�W����
	int m_version;

	bool parse_v3_tag();
//	BYTE* v1tov3(BYTE *ps, DWORD slen, DWORD *pdlen);
//	BYTE* v3tov1(BYTE *ps, DWORD slen, DWORD *pdlen);
};
