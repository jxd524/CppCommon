/*
Model: 字符编码转换类。
Author: 
Date:
Description: 使用 WideCharToMultiByte() 和	
                   MultiByteToWideChar()，进行了简单封装
*/

#pragma once

class CEncoder
{
public:
	CEncoder(void);
	~CEncoder(void);

	void SetWString(const wchar_t*);
	void SetMString(const char*, UINT CP = CP_ACP);

	const wchar_t* GetWString();
	const char* GetMString(UINT);
protected:
	void ClearString();
	
protected:
	const void*		m_initstring; // 记录用户传入的字符串指针，防止错误的delete
	const wchar_t* m_wbuff;
	const char*		m_utf8buff;
	const char*		m_capbuff;
};

unsigned int UTF8Length(const wchar_t *uptr, unsigned int tlen);
void UTF8FromUTF16(const wchar_t *uptr, unsigned int tlen, char *putf, unsigned int len);
unsigned int UTF8CharLength(unsigned char ch);
unsigned int UTF16Length(const char *s, unsigned int len);
unsigned int UTF16FromUTF8(const char *s, unsigned int len, wchar_t *tbuf, unsigned int tlen);
