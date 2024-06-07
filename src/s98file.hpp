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

	// ファイルパス
	char *m_pPath;
	// 読み込んだタグのメモリイメージ
	char *m_pTag;
	// 読み込んだタグのメモリイメージサイズ
	int m_tag_size;
	// タグの文字コード
	s98v3tag::TAG_CODE m_tag_code;
	// タグ情報
	std::list<s98v3tag*> m_pTagList;
	// タグ開始オフセット
	int m_tag_offset;
	// S98バージョン
	int m_version;

	bool parse_v3_tag();
//	BYTE* v1tov3(BYTE *ps, DWORD slen, DWORD *pdlen);
//	BYTE* v3tov1(BYTE *ps, DWORD slen, DWORD *pdlen);
};
