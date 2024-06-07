#pragma once
//////////////////////////////////////////////////////
//
// multibyte -> utf8 convert
//
//////////////////////////////////////////////////////
#include <windows.h>

class MBtoUTF8 {

  public:
	MBtoUTF8();
	MBtoUTF8(LPCSTR);
	~MBtoUTF8();
	bool set_data(LPCSTR);
	LPSTR get_data();

  private:

	LPSTR m_buffer;
};

class UTF8toMB {

  public:
	UTF8toMB();
	UTF8toMB(LPCSTR);
	~UTF8toMB();
	bool set_data(LPCSTR);
	LPSTR get_data();

  private:

	LPSTR m_buffer;
};
