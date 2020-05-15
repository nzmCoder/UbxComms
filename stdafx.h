/****************************************************************************
UBX Comms - Communicate with uBlox NEO-6M GPS device.
Copyright (C) 2020 Norm Moulton
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/

#pragma once

#define _WIN32_WINNT  0x0601
#include <sdkddkver.h>

#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _AFX_ALL_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <afxwin.h>     // MFC core and standard components.
#include <afxext.h>     // MFC extensions.
#include <afxdtctl.h>   // MFC support for Internet Explorer 4 Common Controls.
#include <afxcmn.h>     // MFC support for Windows Common Controls.
