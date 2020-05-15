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
#include "resource.h"
#include "Protocol.h"
#include "Profile.h"

class CUbxCommsDlg : public CDialog
{
public:
   CUbxCommsDlg(CWnd* pParent = nullptr);

   enum { IDD = IDD_MAIN };

protected:
   HICON m_hIcon;

   virtual void DoDataExchange(CDataExchange* pDX);
   virtual BOOL OnInitDialog();

   DECLARE_MESSAGE_MAP()

   // Dialog controls.
   CFont mFont;
   CComboBox mCmboPort;
   CEdit mEditConsole;
   CStatic mStatVersion;
   CStatic mStatUtc;
   CStatic mStatPosition;
   CStatic mStatUbxRaw;
   CStatic mStatOutputFolder;
   CStatic mStatInd;
   CStatic mStatPosEcef;
   CListBox mListRaw;
   CProgressCtrl mProgress;

   int mProgressPos;

   // Access to protocol.
   CProtocol mProtocol;

   // Persistent configuration storage.
   CProfile mProfile;

   CFile mUbxOutFile;

public:
   void RecvMsgCallBack(uint16_t cmdId, uint8_t* dataPtr, uint16_t size);
   void AddTextToConsole(CString str);
   void ClearTextConsole();
   void ProcessMonVerRsp(uint8_t* dataPtr, uint16_t size);
   void ProcessTimeUtcRsp(uint8_t* dataPtr, uint16_t size);
   void ProcessLatLonHeightRsp(uint8_t* dataPtr, uint16_t size);
   void ProcessEarthCenteredPosRsp(uint8_t* dataPtr, uint16_t size);
   void ProcessUbxRawRsp(uint8_t* dataPtr, uint16_t size);
   void UpdateActivityCtrl();

private:
   void ClearAllCtrls();
   CString GetFolder(CString strBasePath);
   void BeginUbxFile();
   void WriteToUbxFile(uint8_t* ptr, uint16_t size);
   void EndUbxFile();

   const int RECV_SERIAL_TIMER_ID = 1;
   const int RECV_SERIAL_TIMER_MSECS = 10;
   const int PROGRESS_RANGE = 500;

public:
   // Framework generated message map functions.
   afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
   afx_msg void OnPaint();
   afx_msg HCURSOR OnQueryDragIcon();

   // User generated message map functions.
   afx_msg void OnTimer(UINT_PTR nIDEvent);
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedCancel();
   afx_msg void OnBnClickedAbout();
   afx_msg void OnCbnDropdownCmboPort();
   afx_msg void OnCbnCloseupCmboPort();
   afx_msg void OnBnClickedSendRxmRawOn();
   afx_msg void OnBnClickedAllMsgsOff();
   afx_msg void OnBnClickedSendMonVer();
   afx_msg void OnBnClickedSendTimeUtc();
   afx_msg void PosLatLonHeight();
   afx_msg void OnBnClickedBtnClr();
   afx_msg void OnBnClickedBtnOutFolder();
   afx_msg void OnBnClickedBtnRecOn();
   afx_msg void OnBnClickedBtnRecOff();
   afx_msg void OnBnClickedBtnNavPosecef();
};
