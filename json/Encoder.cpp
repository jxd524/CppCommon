#include "StdAfx.h"
#include "Encoder.h"

CEncoder::CEncoder(void):
	m_initstring(NULL), m_wbuff(NULL), m_utf8buff(NULL), m_capbuff(NULL)
{
}

CEncoder::~CEncoder(void)
{
	ClearString();
}

void
CEncoder::ClearString(void)
{
	if (m_wbuff != m_initstring && m_wbuff != NULL)
	{
		delete[] m_wbuff;
	}
	if (m_utf8buff != m_initstring && m_utf8buff != NULL)
	{
		delete[] m_utf8buff;
	}
	if (m_capbuff != m_initstring && m_capbuff != NULL)
	{
		delete[] m_capbuff;
	}
	m_initstring = NULL;
	m_wbuff = NULL;
	m_utf8buff = NULL;
	m_capbuff = NULL;
}

void
CEncoder::SetWString(const wchar_t* Str)
{
	ClearString();
	m_initstring = Str;
	m_wbuff = Str;
}
void
CEncoder::SetMString(const char* Str, UINT Cp)
{
	ClearString();

	if (Str && strlen(Str) > 0)
	{
		m_initstring = Str;
		UINT wlen = MultiByteToWideChar(Cp, 0, Str, strlen(Str), NULL, 0);
		wchar_t* pbuff = new wchar_t[wlen + 1];
		if (NULL == pbuff)
		{
			return ;
		}
		memset(pbuff, 0, sizeof(TCHAR) * (wlen + 1));
		MultiByteToWideChar(Cp, 0, Str, strlen(Str), pbuff, wlen);

		m_wbuff = pbuff;
		if (Cp == CP_ACP)
		{
			m_capbuff = Str;
		}
		else
		{
			m_utf8buff = Str;
		}
	}
}

const wchar_t* 
CEncoder::GetWString()
{
	return m_wbuff;
}

const char* 
CEncoder::GetMString(UINT Cp)
{
	if (m_wbuff == NULL)
	{
		return NULL;
	}

	if (Cp == CP_ACP && m_capbuff != NULL)
	{
		return m_capbuff;
	}
	if (Cp != CP_ACP && m_utf8buff != NULL)
	{
		return m_utf8buff;
	}
	char* pbuff = NULL;
	UINT buffsize = 0;

	buffsize = WideCharToMultiByte(Cp, 0, m_wbuff, wcslen(m_wbuff), NULL, 0, NULL, NULL);
	if (buffsize > 0)
	{
		pbuff = new char[buffsize + 1];
		if (pbuff == NULL)
		{
			return NULL;
		}
		memset(pbuff, 0x0, buffsize + 1);
		WideCharToMultiByte(Cp, 0, m_wbuff, wcslen(m_wbuff), pbuff, buffsize, NULL, NULL);
	}

	if (Cp == CP_ACP)
	{
		m_capbuff = pbuff;
	}
	else
	{
		m_utf8buff = pbuff;
	}
	return pbuff;
}

// Scintilla source code edit control
// Functions to handle UTF-8 and UTF-16 strings.
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

enum { SURROGATE_LEAD_FIRST = 0xD800 };
enum { SURROGATE_TRAIL_FIRST = 0xDC00 };
enum { SURROGATE_TRAIL_LAST = 0xDFFF };

unsigned int UTF8Length(const wchar_t *uptr, unsigned int tlen) {
	unsigned int len = 0;
	for (unsigned int i = 0; i < tlen && uptr[i];) {
		unsigned int uch = uptr[i];
		if (uch < 0x80) {
			len++;
		} else if (uch < 0x800) {
			len += 2;
		} else if ((uch >= SURROGATE_LEAD_FIRST) &&
			(uch <= SURROGATE_TRAIL_LAST)) {
			len += 4;
			i++;
		} else {
			len += 3;
		}
		i++;
	}
	return len;
}

void UTF8FromUTF16(const wchar_t *uptr, unsigned int tlen, char *putf, unsigned int len) {
	int k = 0;
	for (unsigned int i = 0; i < tlen && uptr[i];) {
		unsigned int uch = uptr[i];
		if (uch < 0x80) {
			putf[k++] = static_cast<char>(uch);
		} else if (uch < 0x800) {
			putf[k++] = static_cast<char>(0xC0 | (uch >> 6));
			putf[k++] = static_cast<char>(0x80 | (uch & 0x3f));
		} else if ((uch >= SURROGATE_LEAD_FIRST) &&
			(uch <= SURROGATE_TRAIL_LAST)) {
			// Half a surrogate pair
			i++;
			unsigned int xch = 0x10000 + ((uch & 0x3ff) << 10) + (uptr[i] & 0x3ff);
			putf[k++] = static_cast<char>(0xF0 | (xch >> 18));
			putf[k++] = static_cast<char>(0x80 | ((xch >> 12) & 0x3f));
			putf[k++] = static_cast<char>(0x80 | ((xch >> 6) & 0x3f));
			putf[k++] = static_cast<char>(0x80 | (xch & 0x3f));
		} else {
			putf[k++] = static_cast<char>(0xE0 | (uch >> 12));
			putf[k++] = static_cast<char>(0x80 | ((uch >> 6) & 0x3f));
			putf[k++] = static_cast<char>(0x80 | (uch & 0x3f));
		}
		i++;
	}
	putf[len] = '\0';
}

unsigned int UTF8CharLength(unsigned char ch) {
	if (ch < 0x80) {
		return 1;
	} else if (ch < 0x80 + 0x40 + 0x20) {
		return 2;
	} else if (ch < 0x80 + 0x40 + 0x20 + 0x10) {
		return 3;
	} else {
		return 4;
	}
}

unsigned int UTF16Length(const char *s, unsigned int len) {
	unsigned int ulen = 0;
	unsigned int charLen;
	for (unsigned int i=0; i<len;) {
		unsigned char ch = static_cast<unsigned char>(s[i]);
		if (ch < 0x80) {
			charLen = 1;
		} else if (ch < 0x80 + 0x40 + 0x20) {
			charLen = 2;
		} else if (ch < 0x80 + 0x40 + 0x20 + 0x10) {
			charLen = 3;
		} else {
			charLen = 4;
			ulen++;
		}
		i += charLen;
		ulen++;
	}
	return ulen;
}

unsigned int UTF16FromUTF8(const char *s, unsigned int len, wchar_t *tbuf, unsigned int tlen) {
	unsigned int ui=0;
	const unsigned char *us = reinterpret_cast<const unsigned char *>(s);
	unsigned int i=0;
	while ((i<len) && (ui<tlen)) {
		unsigned char ch = us[i++];
		if (ch < 0x80) {
			tbuf[ui] = ch;
		} else if (ch < 0x80 + 0x40 + 0x20) {
			tbuf[ui] = static_cast<wchar_t>((ch & 0x1F) << 6);
			ch = us[i++];
			tbuf[ui] = static_cast<wchar_t>(tbuf[ui] + (ch & 0x7F));
		} else if (ch < 0x80 + 0x40 + 0x20 + 0x10) {
			tbuf[ui] = static_cast<wchar_t>((ch & 0xF) << 12);
			ch = us[i++];
			tbuf[ui] = static_cast<wchar_t>(tbuf[ui] + ((ch & 0x7F) << 6));
			ch = us[i++];
			tbuf[ui] = static_cast<wchar_t>(tbuf[ui] + (ch & 0x7F));
		} else {
			// Outside the BMP so need two surrogates
			int val = (ch & 0x7) << 18;
			ch = us[i++];
			val += (ch & 0x3F) << 12;
			ch = us[i++];
			val += (ch & 0x3F) << 6;
			ch = us[i++];
			val += (ch & 0x3F);
			tbuf[ui] = static_cast<wchar_t>(((val - 0x10000) >> 10) + SURROGATE_LEAD_FIRST);
			ui++;
			tbuf[ui] = static_cast<wchar_t>((val & 0x3ff) + SURROGATE_TRAIL_FIRST);
		}
		ui++;
	}
	return ui;
}
