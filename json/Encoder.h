/*
Model: �ַ�����ת���ࡣ
Author: 
Date:
Description: ʹ�� WideCharToMultiByte() ��	
                   MultiByteToWideChar()�������˼򵥷�װ
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
	const void*		m_initstring; // ��¼�û�������ַ���ָ�룬��ֹ�����delete
	const wchar_t* m_wbuff;
	const char*		m_utf8buff;
	const char*		m_capbuff;
};

unsigned int UTF8Length(const wchar_t *uptr, unsigned int tlen);
void UTF8FromUTF16(const wchar_t *uptr, unsigned int tlen, char *putf, unsigned int len);
unsigned int UTF8CharLength(unsigned char ch);
unsigned int UTF16Length(const char *s, unsigned int len);
unsigned int UTF16FromUTF8(const char *s, unsigned int len, wchar_t *tbuf, unsigned int tlen);
