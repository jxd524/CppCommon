#pragma once

//Regedit
bool RegDelete(HKEY ARootKey, const TCHAR* ApSubPath, const TCHAR* ApName);
bool RegReadValue(HKEY ARootKey, const TCHAR* ApSubPath, const TCHAR* ApName, TCHAR *ApValue, int &AValueLen);
bool RegWriteValue(HKEY ARootKey, const TCHAR* ApSubPath, const TCHAR* ApName, DWORD AType, LPBYTE ApValue, DWORD AValueLen );
bool RegGetValueType(HKEY ARootKey, const TCHAR* ApSubPath, const TCHAR* ApName, DWORD &AType);

bool RegWriteString(HKEY ARottKey, const TCHAR* ApSubPath, const TCHAR* ApName, const TCHAR* ApValue, DWORD AStyle = REG_SZ);

bool IsSoftAutoRunByReg();
bool SetSoftAutoRunByReg(bool bAutoRun, const TCHAR *ApAutRunParam);