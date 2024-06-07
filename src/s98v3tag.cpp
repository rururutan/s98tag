//////////////////////////////////////////////////////////
//
//
//
// $Date: 2005/08/05 02:54:25 $ / $Revision: 1.1 $
//
//////////////////////////////////////////////////////////
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "code_conv.hpp"
#include "s98v3tag.hpp"

s98v3tag::s98v3tag(TAG_CODE code) :
	m_tag_name(NULL),
	m_tag_value(NULL)
{
		m_tag_code = code;
}

s98v3tag::~s98v3tag()
{
	if (m_tag_name)
	  delete [] m_tag_name;
	if (m_tag_value)
	  delete [] m_tag_value;
}

char* s98v3tag::get_tag_name()
{
	return m_tag_name;
}

char* s98v3tag::get_tag_value()
{
	return m_tag_value;
}

bool s98v3tag::set_tag(const char *tag, size_t tag_len)
{
	if (m_tag_name) delete [] m_tag_name;
	m_tag_name = new char[tag_len+1];
	memset(m_tag_name, 0, tag_len + 1);
	strncpy(m_tag_name, tag, tag_len);
	return true;
}

bool s98v3tag::set_value(const char *p_value, size_t value_len)
{
	if (m_tag_value) {
		delete [] m_tag_value;
		m_tag_value = NULL;
	}

	m_tag_value = new char[value_len+1];
	if (!m_tag_value)
	  return false;

	memset(m_tag_value, 0, value_len + 1);
	strncpy(m_tag_value, p_value, value_len);
	return true;
}

bool s98v3tag::set_code(TAG_CODE code)
{
	if (code != m_tag_code) {
		if (m_tag_name) {
			if (code == CODE_ANSI) {
				UTF8toMB um(m_tag_name);
				set_tag(um.get_data(), strlen(um.get_data()));
			} else {
				MBtoUTF8 mu(m_tag_name);
				set_tag(mu.get_data(), strlen(mu.get_data()));
			}
		}
		if (m_tag_value) {
			if (code == CODE_ANSI) {
				UTF8toMB um(m_tag_value);
				set_value(um.get_data(), strlen(um.get_data()));
			} else {
				MBtoUTF8 mu(m_tag_value);
				set_value(mu.get_data(), strlen(mu.get_data()));
			}
		}
		m_tag_code = code;
	}
	return true;
}

