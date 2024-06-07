//////////////////////////////////////////////////////////
//
// 
//
// $Date: 2005/08/05 02:54:25 $ / $Revision: 1.1 $
//
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <memory.h>
#include "getset.h"
#include "code_conv.hpp"
#include "s98file.hpp"
#include "s98device.h"
#include "v3tov1.h"
#include "v1tov3.h"

/* S98 file header */
#define S98_MAGIC_V0	(0x53393830)	/* 'S980' */
#define S98_MAGIC_V1	(0x53393831)	/* 'S981' */
#define S98_MAGIC_V2	(0x53393832)	/* 'S982' */
#define S98_MAGIC_V3	(0x53393833)	/* 'S983' */
#define S98_MAGIC_VZ	(0x5339385A)	/* 'S98Z' */
#define S98_OFS_MAGIC		(0x00)
#define S98_OFS_TIMER_INFO1	(0x04)
#define S98_OFS_TIMER_INFO2	(0x08)
#define S98_OFS_COMPRESSING	(0x0C)
#define S98_OFS_OFSTITLE	(0x10)
#define S98_OFS_OFSDATA		(0x14)
#define S98_OFS_OFSLOOP		(0x18)
#define S98_OFS_DEVICECOUNT	(0x1C)
#define S98_OFS_DEVICEINFO	(0x20)

static unsigned char const utf8_magic[3] = {0xef, 0xbb, 0xbf};

#if defined(WIN32)

#include <windows.h>
static void truncate(const char *filename, long size) {
	HANDLE fh = CreateFile(
		filename,
		GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if(fh == INVALID_HANDLE_VALUE)
	  return;
	SetFilePointer(fh, size, NULL, FILE_BEGIN);
	if(GetLastError() == NO_ERROR)
	  SetEndOfFile(fh);
	CloseHandle(fh);
}

#else

#include <unistd.h>

#endif

s98file::s98file() :
	m_pTag(NULL),
	m_pPath(NULL),
	m_tag_size(0),
	m_tag_offset(0),
	m_tag_code(s98v3tag::CODE_ANSI),
	m_version(3)
{
}

s98file::~s98file()
{
	if (m_pTag)
	  delete [] m_pTag;
	if (m_pPath)
	  delete [] m_pPath;

	std::list<s98v3tag*>::iterator itr = m_pTagList.begin();
	for( ; itr != m_pTagList.end() ; itr++) {
		s98v3tag *tag = *itr;
		delete tag;
	}

}

bool s98file::read(char *path)
{
	int  result(0);
	FILE *fp(NULL);

	if (!path) {
		goto error;
	}

	fp = fopen(path, "rb");
	if (!fp) {
		result = 1;
		goto error;
	}

	{
		size_t path_len = strlen(path) + 1;
		m_pPath = new char[path_len];
		if (!m_pPath) {
			result = 4;
			goto error;
		}
		strncpy(m_pPath, path, path_len);
	}

	char head[0x18];
	if(fread(head, 1, 0x18, fp) != 0x18) {
		result = 2;
		goto error;
	}

	if(memcmp(head, "S98", 3)) {
		result = 4;
		goto error;
	}

	// version check
	if (!memcmp(&head[3], "1", 1)) {
		m_version = 1;
	}

	m_tag_offset = GetDwordLE((Uint8*)&head[0x10]);
	if (m_tag_offset == 0) {
		// tag無し
		goto finally;
	}

	int data_offset = GetDwordLE((Uint8*)&head[0x14]);

	fseek(fp, m_tag_offset, SEEK_SET);

	if (m_version == 3) {
		if(fread(head, 1, 5, fp) != 5) {
			result = 2;
			goto error;
		}

		if(memcmp(head, "[S98]", 5)) {
			result = 3;
			goto error;
		}

		fseek(fp, 0, SEEK_END);
		m_tag_size = ftell(fp);

		fseek(fp, m_tag_offset + 5, SEEK_SET);
		m_tag_size -= m_tag_offset + 5;

		m_pTag = new char[m_tag_size + 1];
		memset(m_pTag, 0, m_tag_size + 1);

		if(fread(m_pTag, 1, m_tag_size, fp) != m_tag_size) {
			result = 2;
			goto error;
		}

		parse_v3_tag();
	} else {
		// V1 or V2
	}

	goto finally;

  error:

	switch(result) {
	  case 1:
		printf("file not open\n");
		break;
	  case 2:
		printf("file read error\n");
		break;
	  case 3:
		printf("s98 file broken\n");
		break;
	  case 4:
		printf("not in s98 format.\n");
		break;
	  default:
		printf("error\n");
		break;
	}

  finally:

	if (fp) fclose(fp);

	return result?false:true;
}

bool s98file::update()
{
	int  result(0);
	FILE *fp(NULL);

	fp = fopen(m_pPath, "r+b");
	if (!fp) {
		result = 1;
		goto error;
	}

	int tag_offset(m_tag_offset);
	if (m_pTagList.size() == 0) {
		tag_offset = 0;
	}

	// ヘッダのタグオフセット更新
	if (m_version == 3) {
		if (m_tag_offset == 0) {
			fseek(fp, 0, SEEK_END);
			m_tag_offset = ftell(fp);
			fseek(fp, S98_OFS_OFSTITLE, SEEK_SET);
			unsigned char num = m_tag_offset & 0xff;
			fwrite(&num, 1, 1,fp);
			num = (m_tag_offset>> 8) & 0xff;
			fwrite(&num, 1, 1,fp);
			num = (m_tag_offset>> 16) & 0xff;
			fwrite(&num, 1, 1,fp);
			num = (m_tag_offset>> 24) & 0xff;
			fwrite(&num, 1, 1,fp);

			// データ終端コマンド存在チェック
			fseek(fp, m_tag_offset -1, SEEK_SET);
			unsigned char eod;
			fread(&eod, 1, 1, fp);
			if (eod != 0xfd) {
				fseek(fp, m_tag_offset, SEEK_SET);
				unsigned char num = 0xfd;
				fwrite(&num, 1, 1, fp);
				m_tag_offset ++;
			}
		}

		// タグ識別子作成
		fseek(fp, m_tag_offset, SEEK_SET);
		fwrite("[S98]", 1, 5, fp);

		// BOM作成
		if (m_tag_code == s98v3tag::CODE_UTF8) {
			fwrite(&utf8_magic[0], 1, 1, fp);
			fwrite(&utf8_magic[1], 1, 1, fp);
			fwrite(&utf8_magic[2], 1, 1, fp);
		}

		// タグ作成
		{
			std::list<s98v3tag*>::iterator itr = m_pTagList.begin();
			for( ; itr != m_pTagList.end() ; itr++) {
				s98v3tag *pTag = *itr;
				fwrite(pTag->get_tag_name(), 1, strlen(pTag->get_tag_name()), fp);
				fwrite("=", 1, 1, fp);
				fwrite(pTag->get_tag_value(), 1, strlen(pTag->get_tag_value()), fp);
				fwrite("\n", 1, 1, fp);
			}
		}
	} else {
		// V1
		if (m_tag_offset == 0 && m_pTagList.size()) {
			m_tag_offset = 0x40;
			fseek(fp, S98_OFS_OFSTITLE, SEEK_SET);
			unsigned char num = m_tag_offset & 0xff;
			fwrite(&num, 1, 1,fp);
			num = (m_tag_offset>> 8) & 0xff;
			fwrite(&num, 1, 1,fp);
			num = (m_tag_offset>> 16) & 0xff;
			fwrite(&num, 1, 1,fp);
			num = (m_tag_offset>> 24) & 0xff;
			fwrite(&num, 1, 1,fp);

			// データ終端コマンド存在チェック
			fseek(fp, 0, SEEK_END);
			m_tag_offset = ftell(fp);
			fseek(fp, m_tag_offset -1, SEEK_SET);
			unsigned char eod;
			fread(&eod, 1, 1, fp);
			if (eod != 0xfd) {
				fseek(fp, m_tag_offset, SEEK_SET);
				unsigned char num = 0xfd;
				fwrite(&num, 1, 1, fp);
				m_tag_offset ++;
			}
		}
	}
	goto finally;

  error:

	switch(result) {
	  case 1:
		printf("file not open\n");
		break;
	  case 2:
		printf("file read error\n");
		break;
	  case 3:
		printf("s98 file broken\n");
		break;
	  default:
		printf("error\n");
		break;
	}

  finally:

	int fsize = ftell(fp);
	if (fp) fclose(fp);

	if (result == 0)
		truncate(m_pPath, fsize);

	return result?false:true;
}

static char* skip_space(char *&ptr)
{
	while (1) {
		if (*ptr != ' ' && *ptr != '\t') break;
		if (*ptr == '\0') break;
		ptr++;
	}
	return ptr;
}

static unsigned char* _utf8inc(const unsigned char* ptr) {
	int inc(0);
	for (int i =0; i < 7; i++) {
		if ((*ptr << i) & 0x80) {
			inc += 1;
		} else {
			if (!inc) inc = 1;
			break;
		}
	}
	return (const_cast<unsigned char*>(ptr + inc));
}

#include <mbstring.h>
static unsigned char* strinc(s98v3tag::TAG_CODE mode, const void* ptr) {
	switch(mode) {
		case 1:
			return _utf8inc(static_cast<const unsigned char*>(ptr));
		case 0:
		default:
			return _mbsinc(static_cast<const unsigned char*>(ptr));
	}
}

bool s98file::parse_v3_tag()
{
	char *work = m_pTag;
	// check for UTF-8's BOM
	if (((unsigned char*)m_pTag)[0] == utf8_magic[0] &&
		((unsigned char*)m_pTag)[1] == utf8_magic[1] &&
		((unsigned char*)m_pTag)[2] == utf8_magic[2]) {
			m_tag_code = s98v3tag::CODE_UTF8;
			work += 3;
	}

	const char *tag_start, *value_start;
	int tag_len, value_len;
	while (*work != 0) {
		while (*work == 0x0a) {
			if (!*work) return true;
			work = (char*)strinc(m_tag_code, work);
		}
		::skip_space(work);

		// tag
		tag_start = work;

		while((*work != 0x20) && (*work != '=')) {
			if (!*work) return true;
			work = (char*)strinc(m_tag_code, work);
		}
		// tag終端
		tag_len = work - tag_start;

		// value
		while(*work == '=') {
			if (!*work) return true;
			work = (char*)strinc(m_tag_code, work);
		}
		::skip_space(work);
		value_start = work;

		while(*work != 0x0a) {
			if (!*work) return true;
			work = (char*)strinc(m_tag_code, work);
		}
		// value終端
		value_len = work - value_start;

		s98v3tag *pTag = new s98v3tag(m_tag_code);
		pTag->set_tag(tag_start, tag_len);
		pTag->set_value(value_start, value_len);
		m_pTagList.push_back(pTag);

		::skip_space(work);
		// 空行飛ばし
		while (*work == 0x0a) {
			if (!*work) return true;
			work = (char*)strinc(m_tag_code, work);
		}
	}

	return true;
}
bool s98file::is_unicode()
{
	return (m_tag_code == s98v3tag::CODE_UTF8);
}

bool s98file::set_code(s98v3tag::TAG_CODE code)
{
	m_tag_code = code;
	std::list<s98v3tag*>::iterator itr = m_pTagList.begin();
	for( ; itr != m_pTagList.end() ; itr++) {
		s98v3tag *tag = *itr;
		tag->set_code(code);
	}
	return true;
}

int s98file::get_tag_count()
{
	return (int)m_pTagList.size();
}

s98v3tag* s98file::get_tag(int count)
{
	std::list<s98v3tag*>::iterator itr = m_pTagList.begin();
	int i(0);
	for (int i =0; i < count; i++) {
		itr++;
	}
	return *itr;
}

s98v3tag* s98file::get_tag(char *tag_name)
{
	if (m_pTagList.size() == 0)
		return NULL;

	std::list<s98v3tag*>::iterator itr = m_pTagList.begin();
	for( ; itr != m_pTagList.end() ; itr++) {
		s98v3tag *tag = *itr;
		if (strnicmp(tag->get_tag_name(), tag_name, strlen(tag->get_tag_name())) == 0)
			return tag;
	}
	return NULL;
}

bool s98file::set_tag(char *tag_name, char *value)
{
	s98v3tag *pTag = get_tag(tag_name);
	if (pTag == NULL) {
		pTag = new s98v3tag(m_tag_code);
		m_pTagList.push_back(pTag);
		pTag->set_tag(tag_name, strlen(tag_name));
	}

	if (m_tag_code == s98v3tag::CODE_UTF8) {
		MBtoUTF8 mu(value);
		pTag->set_value(mu.get_data(), strlen(mu.get_data()));
	} else {
		pTag->set_value(value, strlen(value));
	}
	return true;
}

bool s98file::add_tag(char *tag_name, char *value)
{
	return false;
}

bool s98file::remove_tag(char *tag_name)
{
	s98v3tag *pTag = get_tag(tag_name);
	if (pTag) {
		m_pTagList.remove(pTag);
		return true;
	}
	return false;
}

bool s98file::convert_version(unsigned int version)
{
	if (version <= 3 && version != m_version) {
		FILE *fp(NULL);
		fp = ::fopen(m_pPath, "r+b");
		if (fp) {
			::fseek(fp, 0, SEEK_END);
			int rsize = ::ftell(fp);
			BYTE *rbuf = (BYTE*)::malloc(rsize);
			::fseek(fp, 0, SEEK_SET);
			::fread(rbuf, rsize, 1, fp);

			DWORD dsize;
			BYTE *dbuf;
			if (m_version < 3) {
				dbuf = v1tov3(rbuf, rsize, &dsize);
			} else {
				dbuf = v3tov1(rbuf, rsize, &dsize);
			}

			::fseek(fp, 0, SEEK_SET);
			::fwrite(dbuf, dsize, 1, fp);
			::fclose(fp);
			truncate(m_pPath, dsize);
			return true;
		}
	}
	return false;
}

int s98file::get_version()
{
	return m_version;
}
