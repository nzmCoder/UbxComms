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
#include <cstdint>
#include "UbxCommsDlg.h"
#include "Protocol.h"
#include "Crc.h"
#include "Util.h"


CProtocol::CProtocol(CUbxCommsDlg* pDlg) :
   mDlgPtr(pDlg)
{
   ClearRecvBuff();
}

CProtocol::~CProtocol()
{
   if (mSerial.IsOpen())
   {
      mSerial.Close();
   }
}

void CProtocol::SetupPort(CString strPort)
{
   // First make sure the port starts off as closed.
   if (mSerial.IsOpen())
   {
      mSerial.Close();
   }

   // The default before selecting anything.
   if (strPort != "<None>")
   {
      // Change e.g. "COM1" to "\\\\.\\COM1"
      // This format is "fully qualified" and always works.
      CString str;
      str.Format("\\\\.\\%s", strPort);

      // Open the port and suggest to the driver to use large buffers.
      // But the actual buffer size is constrained by the hardware.
      if (0 != mSerial.Open(str, 4096, 4096))
      {
         // Report error to the user.
         str = "Unable to open port " + strPort;
         AfxMessageBox(str, MB_ICONSTOP);
      }
      else
      {
         // Port opened correctly. Now set attributes.
         mSerial.Setup(CSerial::EBaud9600, CSerial::EData8,
                       CSerial::EParNone, CSerial::EStop1);
         mSerial.SetupHandshaking(CSerial::EHandshakeOff);
         mSerial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking);

         // Clear out any garbage that might be in the buffer.
         mSerial.Purge();
         mRecvInSync = false;
      }
   }
}

void CProtocol::ClearRecvBuff()
{
   memset(mRecvBuff, 0, sizeof(mRecvBuff));
   mRecvPtr = mRecvBuff;
   mRecvLengthField = 0;
   mRecvInSync = false;
}

void CProtocol::ClearSendBuff()
{
   memset(mSendBuff, 0, sizeof(mSendBuff));
}

int CProtocol::CalcRecvLen()
{
   // Use pointer positions to figure out how many bytes so far.
   int length = (int)(mRecvPtr - mRecvBuff);

   return length;
}

void CProtocol::AlignToNextStartFlag()
{
   mRecvInSync = false;
   mRecvLengthField = 0;

   bool bFoundFlag = false;
   int i = 1;

   while (i < CalcRecvLen())
   {
      if (mRecvBuff[i] == START_PATTERN[0])
      {
         // Slide the buffer contents left.
         memmove(mRecvBuff, &mRecvBuff[i], CalcRecvLen() - i);

         // Adjust the insertion pointer accordingly.
         mRecvPtr -= i;

         bFoundFlag = true;
         break;
      }

      ++i;
   }

   if (!bFoundFlag)
   {
      ClearRecvBuff();
   }
}

bool CProtocol::VerifyRecvMsgCrc()
{
   int crcIdx = mRecvLengthField + 6;

   uint16_t crcMsg = (mRecvBuff[crcIdx] << 8) + mRecvBuff[crcIdx + 1];
   uint16_t crcCalc = Crc::Calc16(mRecvBuff + 2, mRecvLengthField + 4);

   return (crcMsg == crcCalc);
}

void CProtocol::SetSendMsgCrc(uint16_t size)
{
   int crcIdx = size - 2;

   uint16_t crcCalc = Crc::Calc16(mSendBuff + 2, size - 4);

   mSendBuff[crcIdx] = (uint8_t)(crcCalc >> 8);
   mSendBuff[crcIdx + 1] = (uint8_t)(crcCalc);
}

void CProtocol::RecvMsg()
{
   MsgFormat_t* pMsg = (MsgFormat_t*)mRecvBuff;
   uint16_t dataSize = pMsg->Length;

   mDlgPtr->RecvMsgCallBack(Util::Reverse16(pMsg->MsgId), pMsg->Data, dataSize);

   ClearRecvBuff();
}

void CProtocol::DisplayMsg(uint16_t msgLength, MsgFormat_t* pMsg)
{
   CString strMsg;
   CString str;

   strMsg.Format("[%03d] ", msgLength);

   uint8_t* pRaw = (uint8_t*)pMsg;

   for (int i = 0; i < msgLength; ++i)
   {
      str.Format("%02X ", pRaw[i]);
      strMsg += str;
   }

   strMsg += "\r\n";

   mDlgPtr->AddTextToConsole(strMsg);
}

void CProtocol::RecvSerialBytes()
{
   DWORD count = 0;
   uint8_t byte = 0;

   // Try to read a byte.
   mSerial.Read(&byte, 1, &count);

   // Loop as long as more chars are available.
   while (count)
   {

#if 0
      // NZM: Introduce intentional errors
      // to test protocol error recovery.
      if (rand() % 500 == 1)
      {
         byte = 0xA7;  // Rogue byte.
      }
#endif

      CString strDb;
      strDb.Format("%02X ", byte);
      OutputDebugString(strDb);

      // Add byte to buffer and advance pointer.
      *mRecvPtr++ = byte;

      mDlgPtr->UpdateActivityCtrl();

      // First sanity check.
      if (CalcRecvLen() > sizeof(mRecvBuff))
      {
         ClearRecvBuff();
      }

      // Check if we have enough chars to look for valid start pattern.
      while (!mRecvInSync && (CalcRecvLen() >= sizeof(START_PATTERN)))
      {
         if (memcmp(mRecvBuff, START_PATTERN, sizeof(START_PATTERN)) == 0)
         {
            // Found the expected start pattern bytes.
            mRecvInSync = true;
         }
         else
         {
            // Attempt to re-sync the frame alignment.
            AlignToNextStartFlag();
         }
      }

      // Check if we have enough chars to get the length field.
      if (mRecvInSync && CalcRecvLen() >= 6)
      {
         mRecvLengthField = (mRecvBuff[5] << 8) + mRecvBuff[4];

         // Sanity check the message length field here.
         // All later processing steps shall hereby be able
         // to trust the length field without further worry.
         if (mRecvLengthField > MAX_DATA_SIZE)
         {
            // Abort!
            OutputDebugString("RecvSerialBytes() had to Abort!\r\n");
            ClearRecvBuff();
         }
      }

      // Check if we are in sync with the header bytes and have
      // received sufficient chars for the complete message yet.
      if (mRecvInSync && (CalcRecvLen() == mRecvLengthField + HDR_SIZE))
      {
         if (VerifyRecvMsgCrc())
         {
            // At this moment we now have all the
            // bytes of a complete, valid message.
            DisplayMsg(mRecvLengthField + HDR_SIZE, (MsgFormat_t*)mRecvBuff);

            // Activate the call-back mechanism to give the
            // higher layer process the received message.
            RecvMsg();

            // Prepare the buffer for the next message.
            ClearRecvBuff();
         }
         else
         {
            // The message failed CRC. Attempt
            // to re-sync the frame alignment.
            AlignToNextStartFlag();
         }
      }

      // Try to read the next byte, if any.
      mSerial.Read(&byte, 1, &count);
   }
}

void CProtocol::SendMsg(uint16_t msgId, uint8_t* dataPtr, uint16_t dataSize)
{
   ClearSendBuff();

   MsgFormat_t* pMsg = (MsgFormat_t*)mSendBuff;
   uint16_t msgLength = dataSize + HDR_SIZE;

   pMsg->Sync[0] = START_PATTERN[0];
   pMsg->Sync[1] = START_PATTERN[1];

   pMsg->MsgId = Util::Reverse16(msgId);
   pMsg->Length = dataSize;

   for (int i = 0; i < dataSize; ++i)
   {
      pMsg->Data[i] = dataPtr[i];
   }

   SetSendMsgCrc(msgLength);

   DisplayMsg(msgLength, (MsgFormat_t*)mSendBuff);

   mSerial.Write(mSendBuff, msgLength);
   Sleep(5);
}

void CProtocol::SendMsgSetCfgPrt()
{
   struct MsgFmtCfgPrt_t
   {
      uint8_t  PortID;
      uint8_t  Reserved0;
      uint16_t TxReady;
      uint32_t Mode;
      uint32_t BaudRate;
      uint16_t InProtoMask;
      uint16_t OutProtoMask;
      uint16_t Reserved4;
      uint16_t Reserved5;
   };

   MsgFmtCfgPrt_t msg = { 0 };
   // Note: Fields not set below should remain zero.

   msg.PortID = 1;          // Setting UART Port 1.
   msg.Mode = 0x08d0;       // N-8-1.
   msg.BaudRate = 9600;     // Bits per second.
   msg.InProtoMask = 0x07;  // Enable all inputs.
   msg.OutProtoMask = 0x01; // Enable only UBX output.

   SendMsg((uint16_t)MSG_CFG_PRT, (uint8_t*)&msg, (uint16_t)sizeof(msg));
}

void CProtocol::SendMsgMonVer()
{
   SendMsg((uint16_t)MSG_MON_VER, 0, 0);
}

void CProtocol::SendMsgEnableRxmRaw()
{
   uint8_t Data[16] =
   {
      0xc8, 0x16, 0x00, 0x00,  // Write addr, 32-bits.
      0x00, 0x00, 0x00, 0x00,  // Flags, 32-bits.
      // Data, 8 bytes.
      0x97, 0x69, 0x21, 0x00,  // Cmd vector address?
      0x00, 0x00, 0x02, 0x10,  // Class/ID of command.
   };

   SendMsg((uint16_t)MSG_UPD_DOWNL, (uint8_t*)Data, (uint16_t)sizeof(Data));
}

void CProtocol::SendMsgEnableRxmSfrb()
{
   uint8_t Data[16] =
   {
      0x0c, 0x19, 0x00, 0x00,  // Write addr, 32-bits.
      0x00, 0x00, 0x00, 0x00,  // Flags, 32-bits.
      // Data, 8 bytes.
      0x83, 0x69, 0x21, 0x00,  // Cmd vector address?
      0x00, 0x00, 0x02, 0x11,  // Class/ID of command.
   };

   SendMsg((uint16_t)MSG_UPD_DOWNL, (uint8_t*)Data, (uint16_t)sizeof(Data));
}

void CProtocol::SendMsgStartPeriodic(uint16_t msgId, uint8_t Rate)
{
   struct MsgFmtCfgPrt_t
   {
      uint16_t MsgId;
      uint8_t  Rate;
   };

   MsgFmtCfgPrt_t msg = { 0 };

   msg.MsgId = Util::Reverse16(msgId);
   msg.Rate = Rate;  // Per second.

   SendMsg((uint16_t)MSG_CFG_MSG, (uint8_t*)&msg, (uint16_t)sizeof(msg));
}

