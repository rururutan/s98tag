#include "code_conv.hpp"

MBtoUTF8::MBtoUTF8() :
	m_buffer(NULL)
{
}

MBtoUTF8::MBtoUTF8(LPCSTR src) :
	m_buffer(NULL)
{
	set_data(src);
}

MBtoUTF8::~MBtoUTF8()
{
	if (m_buffer) delete [] m_buffer;
}

bool MBtoUTF8::set_data(LPCSTR src)
{
	if (!src) return false;
	if (m_buffer) {
		delete [] m_buffer;
		m_buffer = NULL;
	}

	LPWSTR wbuf;
	LPSTR  mbuf;

	// sjis -> unicode
	char * ssrc = (char*)src;
    int ires = ::MultiByteToWideChar(CP_ACP, 0, ssrc, -1, NULL, 0);

	if (ires == 0) {
		return false;
	}

	wbuf = new WCHAR[ires+1];
	ires = ::MultiByteToWideChar(CP_ACP, 0, ssrc, -1, wbuf, ires);

	// unicode -> utf8
	ires = ::WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, NULL, 0, NULL, NULL);

	if (ires == 0) {
		delete [] wbuf;
		return false;
	}

	mbuf = new char[ires];
	ires = ::WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, mbuf, ires, NULL, NULL);

	delete [] wbuf;

	m_buffer = mbuf;

	return true;
}

LPSTR MBtoUTF8::get_data()
{
	return m_buffer;
};

UTF8toMB::UTF8toMB() :
	m_buffer(NULL)
{
}

UTF8toMB::UTF8toMB(LPCSTR src) :
	m_buffer(NULL)
{
	set_data(src);
}

UTF8toMB::~UTF8toMB()
{
	if (m_buffer) delete [] m_buffer;
}

bool UTF8toMB::set_data(LPCSTR src)
{
	if (!src) return false;
	if (m_buffer) {
		delete [] m_buffer;
		m_buffer = NULL;
	}

	LPWSTR wbuf;
	LPSTR  mbuf;

	// utf8 -> unicode
	char * ssrc = (char*)src;
    int ires = ::MultiByteToWideChar(CP_UTF8, 0, ssrc, -1, NULL, 0);

	if (ires == 0) {
		return false;
	}

	wbuf = new WCHAR[ires+1];
	ires = ::MultiByteToWideChar(CP_UTF8, 0, ssrc, -1, wbuf, ires);

	// unicode -> sjis
	ires = ::WideCharToMultiByte(CP_ACP, 0, wbuf, -1, NULL, 0, NULL, NULL);

	if (ires == 0) {
		delete [] wbuf;
		return false;
	}

	mbuf = new char[ires];
	ires = ::WideCharToMultiByte(CP_ACP, 0, wbuf, -1, mbuf, ires, NULL, NULL);

	delete [] wbuf;

	m_buffer = mbuf;

	return true;
}

LPSTR UTF8toMB::get_data()
{
	return m_buffer;
};
