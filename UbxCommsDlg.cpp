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

#include "stdafx.h"
#include "UbxComms.h"
#include "UbxCommsDlg.h"
#include "AboutDlg.h"
#include <map>
#include <string>


BEGIN_MESSAGE_MAP(CUbxCommsDlg, CDialog)
   ON_WM_SYSCOMMAND()
   ON_WM_PAINT()
   ON_WM_QUERYDRAGICON()
   ON_WM_TIMER()
   ON_CBN_DROPDOWN(IDC_CMBO_PORT, &CUbxCommsDlg::OnCbnDropdownCmboPort)
   ON_CBN_CLOSEUP(IDC_CMBO_PORT, &CUbxCommsDlg::OnCbnCloseupCmboPort)
   ON_BN_CLICKED(IDOK, &CUbxCommsDlg::OnBnClickedOk)
   ON_BN_CLICKED(IDCANCEL, &CUbxCommsDlg::OnBnClickedCancel)
   ON_BN_CLICKED(IDC_BTN_ABOUT, &CUbxCommsDlg::OnBnClickedAbout)
   ON_BN_CLICKED(IDC_BTN_UBX_RAW_ON, &CUbxCommsDlg::OnBnClickedSendRxmRawOn)
   ON_BN_CLICKED(IDC_BTN_UBX_ALL_OFF, &CUbxCommsDlg::OnBnClickedAllMsgsOff)
   ON_BN_CLICKED(IDC_BTN_SEND_MON_VER, &CUbxCommsDlg::OnBnClickedSendMonVer)
   ON_BN_CLICKED(IDC_BTN_SEND_TIME_UTC, &CUbxCommsDlg::OnBnClickedSendTimeUtc)
   ON_BN_CLICKED(IDC_BTN_NAV_POSLLH, &CUbxCommsDlg::PosLatLonHeight)
   ON_BN_CLICKED(IDC_BTN_CLR, &CUbxCommsDlg::OnBnClickedBtnClr)
   ON_BN_CLICKED(IDC_BTN_OUT_FOLDER, &CUbxCommsDlg::OnBnClickedBtnOutFolder)
   ON_BN_CLICKED(IDC_BTN_REC_ON, &CUbxCommsDlg::OnBnClickedBtnRecOn)
   ON_BN_CLICKED(IDC_BTN_REC_OFF, &CUbxCommsDlg::OnBnClickedBtnRecOff)
   ON_BN_CLICKED(IDC_BTN_NAV_POSECEF, &CUbxCommsDlg::OnBnClickedBtnNavPosEcef)
END_MESSAGE_MAP()

CUbxCommsDlg::CUbxCommsDlg(CWnd* pParent)
   : CDialog(IDD_MAIN, pParent),
   mProtocol(this)
{
   m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUbxCommsDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_CMBO_PORT, mCmboPort);
   DDX_Control(pDX, IDC_EDIT_CONSOLE, mEditConsole);
   DDX_Control(pDX, IDC_STATIC_VER, mStatVersion);
   DDX_Control(pDX, IDC_STATIC_TIME_UTC, mStatUtc);
   DDX_Control(pDX, IDC_STATIC_POSITION, mStatPosition);
   DDX_Control(pDX, IDC_STATIC_POSECEF, mStatPosEcef);
   DDX_Control(pDX, IDC_STATIC_UBX_RAW, mStatUbxRaw);
   DDX_Control(pDX, IDC_STATIC_OUT_FOLDER, mStatOutputFolder);
   DDX_Control(pDX, IDC_STATIC_REC_IND, mStatInd);
   DDX_Control(pDX, IDC_PROGRESS, mProgress);
   DDX_Control(pDX, IDC_LIST_RAWDATA, mListRaw);
}

BOOL CUbxCommsDlg::OnInitDialog()
{
   CDialog::OnInitDialog();

   // Add "About..." menu item to system menu.

   // IDM_ABOUTBOX must be in the system command range.
   ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
   ASSERT(IDM_ABOUTBOX < 0xF000);

   CMenu* pSysMenu = GetSystemMenu(FALSE);
   if (pSysMenu != nullptr)
   {
      BOOL bNameValid;
      CString strAboutMenu;
      bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
      ASSERT(bNameValid);
      if (!strAboutMenu.IsEmpty())
      {
         pSysMenu->AppendMenu(MF_SEPARATOR);
         pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
      }
   }

   // Set the icon for this dialog.
   SetIcon(m_hIcon, TRUE);

   // Format the protocol message window with a fixed width font.
   mFont.CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 0, 0,
      DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,
      DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Fixedsys");
   mEditConsole.SetFont(&mFont, TRUE);

   PrepareListCtrl();

   // Clear the static fields.
   ClearAllCtrls();
   mStatVersion.SetWindowText("");
   mStatInd.SetWindowText(STR_REC_OFF);

   // Start the timer(s).
   SetTimer(RECV_SERIAL_TIMER_ID, RECV_SERIAL_TIMER_MSECS, NULL);
#ifdef FEATURE_SEND_TEST
   SetTimer(SEND_SERIAL_TIMER_ID, SEND_SERIAL_TIMER_MSECS, NULL);
#endif 

   OutputDebugString("Started Serial Timer(s).\r\n");

   // Set up access to persistent data.
   mProfile.Init("UbxComms.xml", true);

   // Set the COM port.
   CString str = mProfile.GetProfileStr("Config", "ComPort", "COM1");
   mCmboPort.AddString(str);
   mCmboPort.SelectString(-1, str);
   mProtocol.SetupPort(str);

   str = mProfile.GetProfileStr("Config", "OutputFolder", "C:\\Projects");
   mStatOutputFolder.SetWindowText(" " + str);

   // Set range just to give reasonable
   // movement as chars are received.
   mProgress.SetRange(0, PROGRESS_RANGE);
   mProgressPos = 0;

   // Make best effort to turn off NMEA messages.
   // May not work if comms not configured yet.
   mProtocol.SendMsgSetCfgPrt();

   return TRUE;
}

void CUbxCommsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
   if ((nID & 0xFFF0) == IDM_ABOUTBOX)
   {
      CAboutDlg dlgAbout;
      dlgAbout.DoModal();
   }
   else
   {
      CDialog::OnSysCommand(nID, lParam);
   }
}

void CUbxCommsDlg::OnPaint()
{
   if (IsIconic())
   {
      CPaintDC dc(this); // Device context for painting.

      SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

      // Center icon in client rectangle.
      int cxIcon = GetSystemMetrics(SM_CXICON);
      int cyIcon = GetSystemMetrics(SM_CYICON);
      CRect rect;
      GetClientRect(&rect);
      int x = (rect.Width() - cxIcon + 1) / 2;
      int y = (rect.Height() - cyIcon + 1) / 2;

      // Draw the icon.
      dc.DrawIcon(x, y, m_hIcon);
   }
   else
   {
      CDialog::OnPaint();
   }
}

HCURSOR CUbxCommsDlg::OnQueryDragIcon()
{
   return static_cast<HCURSOR>(m_hIcon);
}

void CUbxCommsDlg::OnBnClickedOk()
{
   // Prevent inadvertent application close.
}

void CUbxCommsDlg::OnBnClickedCancel()
{
   CDialog::OnCancel();
}

void CUbxCommsDlg::OnBnClickedAbout()
{
   CAboutDlg dlg;
   dlg.DoModal();
}

void CUbxCommsDlg::OnCbnDropdownCmboPort()
{
   // Populate the drop-down with only available com ports.
   mCmboPort.ResetContent();

   static const int DEVICE_BUFF_SIZE = 0xFFFF;
   char* szDevices = new char[DEVICE_BUFF_SIZE]();
   unsigned long dwChars = QueryDosDevice(NULL, szDevices, DEVICE_BUFF_SIZE);
   char* ptr = szDevices;

   // QueryDosDevice returns the com ports, but not in the correct order.
   // Add them to a map, so they can be read off in the proper order.
   std::map<int, std::string> comPortMap;

   while (dwChars)
   {
      std::string strTest(ptr);

      int findPos = strTest.find("COM");

      if (findPos != std::string::npos)
      {
         strTest.erase(findPos, 3);

         int port = atoi(strTest.c_str());
         comPortMap[port] = strTest;
      }

      char* temp_ptr = strchr(ptr, 0);
      dwChars -= (DWORD)((temp_ptr - ptr) / sizeof(char) + 1);
      ptr = temp_ptr + 1;
   }

   // If anything was added to the map.
   if (!comPortMap.empty())
   {
      for (std::map<int, std::string>::iterator comItr = comPortMap.begin(); comItr
         != comPortMap.end(); ++comItr)
      {
         std::string str = comItr->second;
         str = "COM" + str;
         mCmboPort.AddString(str.c_str());
      }
   }
   else
   {
      // Don't let the combo box be empty.
      CString str = "<None>";
      mCmboPort.AddString(str);
   }

   delete[]szDevices;
}

void CUbxCommsDlg::OnCbnCloseupCmboPort()
{
   // Get user selected port string value. E.g. "COM1".
   CString str;
   mCmboPort.GetWindowText(str);

   if (str.IsEmpty())
   {
      // Don't let the combo box be empty.
      str = "<None>";
      mCmboPort.AddString(str);
      mCmboPort.SelectString(-1, str);
   }
   else
   {
      mProtocol.SetupPort(str);

      // Turn off the GPS NMEA messages.
      mProtocol.SendMsgSetCfgPrt();
   }

   mProfile.WriteProfileStr("Config", "ComPort", str);
}

void CUbxCommsDlg::OnTimer(UINT_PTR nIDEvent)
{
   if (nIDEvent == RECV_SERIAL_TIMER_ID)
   {
      mProtocol.RecvSerialBytes();
   }
#ifdef FEATURE_SEND_TEST
   else if (nIDEvent == SEND_SERIAL_TIMER_ID)
   {
      mProtocol.SendMsgMonVer();
   }
#endif

   CDialog::OnTimer(nIDEvent);
}

void CUbxCommsDlg::PrepareListCtrl()
{
   // The trailing spaces are used to define the column widths.
   mListRaw.InsertColumn(0, "SV  ");
   mListRaw.InsertColumn(1, "Carrier Phase (m)  ");
   mListRaw.InsertColumn(2, "Pseudorange (m)    ");
   mListRaw.InsertColumn(3, "Doppler (Hz) ");
   mListRaw.InsertColumn(4, "Qual   ");
   mListRaw.InsertColumn(5, "db Hz  ");
   mListRaw.InsertColumn(6, "Lock");

   mListRaw.SetExtendedStyle(mListRaw.GetExtendedStyle()
      | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

   mListRaw.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
   mListRaw.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
   mListRaw.SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);
   mListRaw.SetColumnWidth(3, LVSCW_AUTOSIZE_USEHEADER);
   mListRaw.SetColumnWidth(4, LVSCW_AUTOSIZE_USEHEADER);
   mListRaw.SetColumnWidth(5, LVSCW_AUTOSIZE_USEHEADER);
   mListRaw.SetColumnWidth(6, 40);  // Fixed width will avoid horizontal scroll bar.

   // Set alignment of all columns to right-justified.
   LVCOLUMN col;
   col.mask = LVCF_FMT;
   col.fmt = LVCFMT_RIGHT;
   mListRaw.SetColumn(0, &col);
   mListRaw.SetColumn(1, &col);
   mListRaw.SetColumn(2, &col);
   mListRaw.SetColumn(3, &col);
   mListRaw.SetColumn(4, &col);
   mListRaw.SetColumn(5, &col);
   mListRaw.SetColumn(6, &col);
}

void CUbxCommsDlg::AddTextToConsole(CString str)
{
   CString strConsole;
   mEditConsole.GetWindowText(strConsole);

   strConsole += str;

   // For performance reasons, don't allow console text to grow too long.
   strConsole = strConsole.Right(0xFFF);

   mEditConsole.SetWindowText(strConsole);

   // This puts the window scroll location
   // at the bottom so new text can be seen.
   mEditConsole.SetSel(0, -1);
   mEditConsole.SetSel(-1, -1);
}

void CUbxCommsDlg::ClearTextConsole()
{
   mEditConsole.SetWindowText("");
}

void CUbxCommsDlg::BeginUbxFile()
{
   CString strFileName = mProfile.GetProfileStr("Config", "OutputFolder", "C:\\Projects")
      + "\\uBlox" + GetRinexNameFmt() + ".ubx";

   if (!mUbxOutFile.Open(strFileName, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
   {
      CString str;
      str.Format("Cannot write UBX file: %s", strFileName);
      AfxMessageBox(str);
      return;
   }
}

CString CUbxCommsDlg::GetRinexNameFmt()
{
   // Returns a name that encodes the date and time
   // similar to the RINEX naming convention, DDDHMM-YY:
   // DDD - day of the year,
   // H - letter indicating hour of the day,
   // MM - minute,
   // YY - year.

   CTime time(CTime::GetCurrentTime());
   tm tmGmt;
   time.GetGmtTm(&tmGmt);

   int day = tmGmt.tm_yday + 1;
   char hour = tmGmt.tm_hour + 'a';
   int minute = tmGmt.tm_min;
   int year = tmGmt.tm_year - 100;

   CString strFilename;
   strFilename.Format("%03d%c%02d-%02d", day, hour, minute, year);

   return strFilename;
}

void CUbxCommsDlg::WriteToUbxFile(uint8_t* ptr, uint16_t size)
{
   // Only write if the file is open.
   if ((mUbxOutFile.m_hFile != 0) && (mUbxOutFile.m_hFile != INVALID_HANDLE_VALUE))
   {
      // The passed in ptr points to the start of payload.
      // The passed in size indicates the payload size.
      // This function wants to write the entire packet,
      // including the 6 byte header and 2 byte checksum.
      mUbxOutFile.Write(ptr - HDR_SIZE, size + HDR_SIZE + CHK_SIZE);
   }
}

void CUbxCommsDlg::EndUbxFile()
{
   // Try to close whether open or not.
   mUbxOutFile.Close();
}

void CUbxCommsDlg::RecvMsgCallBack(uint16_t cmdId, uint8_t* dataPtr, uint16_t size)
{
   switch (cmdId)
   {
   case MSG_ACK:
      OutputDebugString("\r\nMSG_ACK\r\n");
      break;

   case MSG_NACK:
      OutputDebugString("\r\nMSG_NACK\r\n");
      break;

   case MSG_MON_VER:
      OutputDebugString("\r\nMSG_MON_VER\r\n");
      ProcessMonVerRsp(dataPtr, size);
      break;

   case MSG_TIME_UTC:
      OutputDebugString("\r\nMSG_TIME_UTC\r\n");
      ProcessTimeUtcRsp(dataPtr, size);
      break;

   case MSG_RXM_RAW:
      OutputDebugString("\r\nMSG_RXM_RAW\r\n");
      WriteToUbxFile(dataPtr, size);
      ProcessUbxRawRsp(dataPtr, size);
      break;

   case MSG_RXM_SFRB:
      OutputDebugString("\r\nMSG_RXM_SFRB\r\n");
      WriteToUbxFile(dataPtr, size);
      break;

   case MSG_POS_LLH:
      OutputDebugString("\r\nMSG_POS_LLH\r\n");
      ProcessLatLonHeightRsp(dataPtr, size);
      break;

   case MSG_POS_ECEF:
      OutputDebugString("\r\nMSG_POS_ECEF\r\n");
      ProcessEarthCenteredPosRsp(dataPtr, size);
      break;

   default:
      CString str;
      str.Format("\r\nMSG_0x%04X\r\n", cmdId);
      OutputDebugString(str);
      break;
   }
}

void CUbxCommsDlg::OnBnClickedSendMonVer()
{
   mProtocol.SendMsgMonVer();
}

void CUbxCommsDlg::OnBnClickedSendRxmRawOn()
{
   mProtocol.SendMsgEnableRxmRaw();
   mProtocol.SendMsgEnableRxmSfrb();
   mProtocol.SendMsgStartPeriodic(MSG_RXM_RAW, 1);
   mProtocol.SendMsgStartPeriodic(MSG_RXM_SFRB, 1);
}

void CUbxCommsDlg::OnBnClickedSendTimeUtc()
{
   mProtocol.SendMsgStartPeriodic(MSG_TIME_UTC, 1);
}

void CUbxCommsDlg::PosLatLonHeight()
{
   mProtocol.SendMsgStartPeriodic(MSG_POS_LLH, 1);
}

void CUbxCommsDlg::OnBnClickedBtnNavPosEcef()
{
   mProtocol.SendMsgStartPeriodic(MSG_POS_ECEF, 1);
}

void CUbxCommsDlg::OnBnClickedAllMsgsOff()
{
   mProtocol.SendMsgStartPeriodic(MSG_RXM_RAW, 0);
   mProtocol.SendMsgStartPeriodic(MSG_RXM_SFRB, 0);
   mProtocol.SendMsgStartPeriodic(MSG_TIME_UTC, 0);
   mProtocol.SendMsgStartPeriodic(MSG_POS_LLH, 0);
   mProtocol.SendMsgStartPeriodic(MSG_POS_ECEF, 0);
}

void CUbxCommsDlg::ProcessMonVerRsp(uint8_t* dataPtr, uint16_t size)
{
   // NEO-6M v7.03 actually uses this format of MON-VER as
   // specified in the uBlox-8 Protocol description.
   struct MsgFmt_t
   {
      uint8_t SwRev[30];
      uint8_t HwVer[10];
   };

   MsgFmt_t* fmtPtr = (MsgFmt_t*)dataPtr;

   CString str;
   str.Format(" SW: %s, HW: %s", fmtPtr->SwRev, fmtPtr->HwVer);

   mStatVersion.SetWindowText(str);
}

void CUbxCommsDlg::ProcessTimeUtcRsp(uint8_t* dataPtr, uint16_t size)
{
   struct MsgFmt_t
   {
      uint32_t iTOW;
      uint32_t tAcc;
      int32_t  nano;
      uint16_t year;
      uint8_t  month;
      uint8_t  day;
      uint8_t  hour;
      uint8_t  min;
      uint8_t  sec;
      uint8_t  valid;
   };

   const char MONTH[13][4] =
   {
      "---", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
   };

   MsgFmt_t* fmtPtr = (MsgFmt_t*)dataPtr;

   CString str;
   str.Format(" %04d-%03s-%02d %02d:%02d:%02d",
      fmtPtr->year, MONTH[fmtPtr->month], fmtPtr->day,
      fmtPtr->hour, fmtPtr->min, fmtPtr->sec);

   mStatUtc.SetWindowText(str);
}

void CUbxCommsDlg::ProcessLatLonHeightRsp(uint8_t* dataPtr, uint16_t size)
{
   struct MsgFmt_t
   {
      uint32_t iTOW;
      int32_t  lon;
      int32_t  lat;
      int32_t  height;
      int32_t  hMSL;
      uint32_t hAcc;
      uint32_t vAcc;
   };

   MsgFmt_t* fmtPtr = (MsgFmt_t*)dataPtr;

   double lat = (double)fmtPtr->lat / 1e+7;
   char ns = 'N';

   if (lat < 0)
   {
      ns = 'S';
      lat = -lat;
   }

   double lon = (double)fmtPtr->lon / 1e+7;
   char ew = 'E';

   if (lon < 0)
   {
      ew = 'W';
      lon = -lon;
   }

   double altitude = (double)fmtPtr->hMSL / 1e+3;
   double errHorz = (double)fmtPtr->hAcc / 1e+3;
   double errVert = (double)fmtPtr->vAcc / 1e+3;

   CString str;
   str.Format(" %c%08.5f %c%09.5f Alt:%4.1fm,  (Err %2.1fm, %2.1fm)",
      ns, lat, ew, lon, altitude, errHorz, errVert);

   mStatPosition.SetWindowText(str);
}

void CUbxCommsDlg::ProcessEarthCenteredPosRsp(uint8_t* dataPtr, uint16_t size)
{
   struct MsgFmt_t
   {
      uint32_t iTOW;
      int32_t  ecefX;
      int32_t  ecefY;
      int32_t  ecefZ;
      uint32_t pAcc;
   };

   MsgFmt_t* fmtPtr = (MsgFmt_t*)dataPtr;

   double x = (double)fmtPtr->ecefX / 1e+2;
   double y = (double)fmtPtr->ecefY / 1e+2;
   double z = (double)fmtPtr->ecefZ / 1e+2;
   double error = (double)fmtPtr->pAcc / 1e+2;

   CString str;
   str.Format(" X:%011.2fm  Y:%011.2fm  Z:%011.2fm (Err %2.2fm)",
      x, y, z, error);

   mStatPosEcef.SetWindowText(str);
}

void CUbxCommsDlg::ProcessUbxRawRsp(uint8_t* dataPtr, uint16_t size)
{
   // Message header.
   struct MsgFmtHdr_t
   {
      uint32_t iTOW;
      int16_t  week;
      uint8_t  numSV;
      uint8_t  pad;
   };

   // Repeated portion for each SV.
   struct MsgFmtSV_t
   {
      double  cpMes;
      double  prMes;
      float   doMes;
      uint8_t sv;
      int8_t  mesQI;
      int8_t  cno;
      uint8_t lli;
   };

   MsgFmtHdr_t* fmtPtr = (MsgFmtHdr_t*)dataPtr;

   CString str;
   str.Format(" iTOW = %d, week = %d, numSV = %d",
      fmtPtr->iTOW, fmtPtr->week, fmtPtr->numSV);

   // Output the header info.
   mStatUbxRaw.SetWindowText(str);

   // Remember the count of SVs.
   uint8_t numSV = fmtPtr->numSV;
   if (numSV > 16) numSV = 16;

   // Now skip beyond header to the first SV frame.
   dataPtr += sizeof(MsgFmtHdr_t);
   MsgFmtSV_t* fmtSvPtr = (MsgFmtSV_t*)dataPtr;

   mListRaw.DeleteAllItems();
   for (size_t i = 0; i < numSV; ++i)
   {
      CString str;

      // Add items on a row of the list ctrl.
      str.Format("%02d", fmtSvPtr->sv);
      int item = mListRaw.InsertItem(0, str);

      str.Format("%1.3f", fmtSvPtr->cpMes);
      mListRaw.SetItem(item, 1, LVIF_TEXT, str, 0, 0, 0, 0);

      str.Format("%1.3f", fmtSvPtr->prMes);
      mListRaw.SetItem(item, 2, LVIF_TEXT, str, 0, 0, 0, 0);

      str.Format("%1.3f", fmtSvPtr->doMes);
      mListRaw.SetItem(item, 3, LVIF_TEXT, str, 0, 0, 0, 0);

      str.Format("%d", fmtSvPtr->mesQI);
      mListRaw.SetItem(item, 4, LVIF_TEXT, str, 0, 0, 0, 0);

      str.Format("%d", fmtSvPtr->cno);
      mListRaw.SetItem(item, 5, LVIF_TEXT, str, 0, 0, 0, 0);

      str.Format("%d", fmtSvPtr->lli);
      mListRaw.SetItem(item, 6, LVIF_TEXT, str, 0, 0, 0, 0);

      fmtSvPtr += 1;
   }
}

void CUbxCommsDlg::ClearAllCtrls()
{
   mStatUtc.SetWindowText("");
   mStatUbxRaw.SetWindowText("");
   mStatPosition.SetWindowText("");
   mStatPosEcef.SetWindowText("");
   mEditConsole.SetWindowText("");
   mListRaw.DeleteAllItems();
   mProgress.SetPos(0);
   mProgressPos = 0;
}

void CUbxCommsDlg::OnBnClickedBtnClr()
{
   ClearAllCtrls();
}

void CUbxCommsDlg::UpdateActivityCtrl()
{
   ++mProgressPos;

   if (mProgressPos > PROGRESS_RANGE) mProgressPos = 0;
   mProgress.SetPos(mProgressPos);
}

void CUbxCommsDlg::OnBnClickedBtnOutFolder()
{
   // End any file that may be in progress.
   EndUbxFile();

   CString str = mProfile.GetProfileStr("Config", "OutputFolder", "C:\\Projects");
   str = GetFolder(str);
   if (!str.IsEmpty())
   {
      mProfile.WriteProfileStr("Config", "OutputFolder", str);
      mStatOutputFolder.SetWindowText(" " + str);
   }
}

CString CUbxCommsDlg::GetFolder(CString strFolderPath)
{
   CFolderPickerDialog dlgPickFolder;

   dlgPickFolder.m_ofn.lpstrTitle = "Select a folder for data storage";
   dlgPickFolder.m_ofn.lpstrInitialDir = strFolderPath;
   if (dlgPickFolder.DoModal() == IDOK)
   {
      strFolderPath = dlgPickFolder.GetPathName();
   }
   return strFolderPath;
}

void CUbxCommsDlg::OnBnClickedBtnRecOn()
{
   BeginUbxFile();
   mStatInd.SetWindowText(STR_REC_ON);
}

void CUbxCommsDlg::OnBnClickedBtnRecOff()
{
   EndUbxFile();
   mStatInd.SetWindowText(STR_REC_OFF);
}

