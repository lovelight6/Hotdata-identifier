// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__CB91CF07_6B4A_4EC4_AF44_B30C95A4B2C5__INCLUDED_)
#define AFX_STDAFX_H__CB91CF07_6B4A_4EC4_AF44_B30C95A4B2C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "mysql.h"
#include "winsock.h"   // 如果编译出错,则把该行放到#include "mysql.h"之前
#include <afxsock.h>		// MFC socket extensions
#pragma comment(lib,"libmySQL.lib")   // 如果在附加依赖项里已增加,则就不要添加了
#define WM_USER_NEW_MESSAGE			WM_USER+0x100
#define WM_USER_NEW_MESSAGE2		WM_USER+0x101
#define DBACCOUNT "root"
#define DBPASSWORD "Ailiang"
#define DBNAME "cm"
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__CB91CF07_6B4A_4EC4_AF44_B30C95A4B2C5__INCLUDED_)
