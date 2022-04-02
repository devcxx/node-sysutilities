/**
 *  Copyright 2008-2009 Cheng Shi.  All rights reserved.
 *  Email: shicheng107@hotmail.com
 */

#ifndef STRINGPROCESS_H
#define STRINGPROCESS_H

#include "RegExp.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <comutil.h>
#pragma warning(push)
#pragma warning(disable: 4127)
#include <atlcomtime.h>
#pragma warning(pop)
#define _import __declspec(dllexport)
using namespace std;

_import inline wstring Trim(const wstring &source, const wstring &targets);
_import
_import inline bool PrepareString(wchar_t *dest, size_t *size, const wstring &src);
_import
_import inline wstring ReplaceString(const wstring &srcStr, const wstring &oldStr, const wstring &newStr);
_import
_import inline int StringToInteger(const wstring &number);
_import
_import inline wstring LowerString(const wstring &text);
_import
_import inline wstring UpperString(const wstring &text);
_import
_import inline wstring GetAnchorText(const wstring &anchor);
_import
_import inline wstring GetAnchorLink(const wstring &anchor);
_import
_import inline bool SeparateString(const wstring &content, const wstring &delimiter, vector<wstring> &result);
_import
_import inline wstring URLEncoding(const wstring &keyword, bool convertToUTF8 = true);
_import
_import inline unsigned int GetSeparateKeywordMatchGrade(const wstring &source, const wstring &keyword);
_import
_import inline unsigned int GetKeywordMatchGrade(const wstring &source, const wstring & keyword);
_import
_import inline wstring GetDateString(const COleDateTime &time, const wstring &separator = L"-", bool align = true);
_import
_import inline wstring GetDateString(int dayOffset, const wstring &separator = L"-", bool align = true);
_import
_import inline wstring GetTimeString(const COleDateTime &time, const wstring &separator = L":", bool align = true);
_import
_import inline wstring MD5(const wstring &text);
_import
_import inline wstring FilterFileName(const wstring &name);
_import
_import inline wstring GetMagic(unsigned int length);
_import
_import inline wstring GetHost(const wstring &url);
_import
_import inline wstring GetValidFileName(const wstring &fileName);
_import
_import inline string WidestringToString(const std::wstring& wstr);

#endif // STRINGPROCESS_H
