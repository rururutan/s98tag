#pragma once
//////////////////////////////////////////////////////////
//
//
//
// $Date: 2005/08/05 02:54:25 $ / $Revision: 1.1 $
//
//////////////////////////////////////////////////////////

class s98v3tag {

  public:

	enum TAG_CODE {
		CODE_ANSI,
		CODE_UTF8
	};

	s98v3tag(TAG_CODE code=CODE_ANSI);
	~s98v3tag();
	char* get_tag_name();
	char* get_tag_value();
	bool set_tag(const char *tag, size_t tag_len);
	bool set_value(const char *p_value, size_t value_len);
	bool set_code(TAG_CODE code);

  private:

	char *m_tag_name;
	char *m_tag_value;
	TAG_CODE m_tag_code;
};

