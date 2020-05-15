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
#include <cstdint>
#include "Serial.h"
#include "MsgFormat.h"

// Fwd declaration.
class CUbxCommsDlg;

class CProtocol
{
public: // Methods.
   CProtocol(CUbxCommsDlg* pDlg);
   ~CProtocol();

   void SetupPort(CString strPort);
   void RecvSerialBytes();

   void SendMsg(uint16_t cmdId, uint8_t* dataPtr, uint16_t size);
   void SendMsgMonVer();
   void SendMsgSetCfgPrt();
   void SendMsgEnableRxmRaw();
   void SendMsgEnableRxmSfrb();
   void SendMsgStartPeriodic(uint16_t msgId, uint8_t Rate);
   
private: // Methods.
   void ClearRecvBuff();
   void ClearSendBuff();

   void RecvMsg();

   void AlignToNextStartFlag();
   bool VerifyRecvMsgCrc();
   int CalcRecvLen();

   void SetSendMsgCrc(uint16_t size);

   void DisplayMsg(uint16_t msgLength, MsgFormat_t* pMsg);

private: // Data.
   // Access to the parent view class dialog.
   CUbxCommsDlg* mDlgPtr;

   // Serial port access.
   CSerial mSerial;

   // Send buffer.
   uint8_t mSendBuff[sizeof(MsgFormat_t)];

   // Receive buffer.
   uint8_t mRecvBuff[sizeof(MsgFormat_t)];
   uint8_t* mRecvPtr;
   uint16_t mRecvLengthField;
   bool mRecvInSync;

   const uint8_t START_PATTERN[2] = { 0xB5, 0x62 };
};
