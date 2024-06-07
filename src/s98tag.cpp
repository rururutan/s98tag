//////////////////////////////////////////////////////////
//
// 
//
// $Date: 2005/08/05 02:54:25 $ / $Revision: 1.1 $
//
//////////////////////////////////////////////////////////

#include <stdio.h>
#include "code_conv.hpp"
#include "s98file.hpp"

void usage()
{
	printf("S98tag v0.3\n");
	printf("usage: s98tag option filename\n\n");
	printf(" option:\n");
	printf("  -i\t\t: tag information\n");
	printf("  -aTag=Value\t: tag write\n");
	printf("  -aTag=\t: tag remove\n");
	printf("  -u\t\t: convert utf-8 tag\n");
	printf("  -m\t\t: convert multi-byte tag\n");
	printf("  -1\t\t: convert V1 to V3\n");
	printf("  -3\t\t: convert V3 to V1\n");
}

void set_tag(s98file *pS98, char *order)
{
	char *tag, *value;
	size_t tag_len, value_len;
	int cmd = 0;

	char *work = order + 2;
	tag = work;

	while(*work!=0 && *work!='=')
		work++;
	if (*work == 0) return;

	tag_len = work - tag;
	value = work +1;
	if (*value == 0) {
		cmd = 1;
	} else {
		while(*work!=0)
			work++;		
			value_len = work - value;
	}

	char *tag_str = new char [tag_len + 1];
	memset(tag_str, 0, tag_len+1);
	strncpy(tag_str, tag, tag_len);
	if (cmd) {
		pS98->remove_tag(tag_str);
	} else {
		char *value_str = new char [value_len + 1];
		memset(value_str, 0, value_len+1);
		strncpy(value_str, value, value_len);
		pS98->set_tag(tag_str, value_str);
		delete [] value_str;
	}
	delete [] tag_str;
}

void info_tag(s98file *pS98)
{
	printf("[Header information]\n");
	printf("Version:%d\n", pS98->get_version());

	printf("[Tag information]\n");
	int count = pS98->get_tag_count();

	if (!count) {
		printf("Tag information doesn't exist.\n");
	}

	printf("Character-code:");
	if (pS98->is_unicode())
		printf("utf8\n");
	else
		printf("ansi\n");

	for (int i=0; i < count; i++) {
		s98v3tag *pTag = pS98->get_tag(i);
		if (pTag) {
			if(pS98->is_unicode()) {
				UTF8toMB u8tag(pTag->get_tag_name()), u8value(pTag->get_tag_value());
				printf("%s = %s\n", u8tag, u8value);
			} else {
				printf("%s = %s\n", pTag->get_tag_name(), pTag->get_tag_value());
			}
		}
	}
}

void convert_ver(s98file *pS98, int version)
{
	switch(version) {
	  case 1:
	  case 3:
		pS98->convert_version(version);
		break;
	  default:
		break;
	}
}

void convert_code(s98file *pS98, s98v3tag::TAG_CODE code)
{
	pS98->set_code(code);
}

void parse_option(char *order, s98file *pS98)
{
	switch(order[1]) {
		case '1':
			convert_ver(pS98, 1);
			break;
		case '3':
			convert_ver(pS98, 3);
			break;
		case 'a':
		case 'A':
			set_tag(pS98, order);
			pS98->update();
			break;
		case 'i':
		case 'I':
			info_tag(pS98);
			break;
		case 'u':
		case 'U':
			convert_code(pS98, s98v3tag::CODE_UTF8);
			pS98->update();
			break;
		case 'm':
		case 'M':
			convert_code(pS98, s98v3tag::CODE_ANSI);
			pS98->update();
			break;
		default:
			break;
	}
}

int main (int argc, char **argv)
{
	if (argc < 3 || argv[1][0] != '-') {
		usage();
		return 1;
	}

	for (int i = 2; i < argc; i++) {
		s98file *pS98 = new s98file;
		if (pS98->read(argv[i]) == true) {
			parse_option(argv[1], pS98);
		}
		delete pS98;
	}
	return 0;
}


