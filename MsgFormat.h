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

const uint16_t HDR_SIZE = 8;
const uint16_t MAX_DATA_SIZE = 512;

#pragma pack(1)
struct MsgFormat_t
{
   uint8_t  Sync[2];
   uint16_t MsgId;
   uint16_t Length;
   uint8_t  Data[MAX_DATA_SIZE];
};

enum
{
   MSG_ACK       = 0x0501,
   MSG_NACK      = 0x0500,
   MSG_MON_VER   = 0x0A04,
   MSG_POS_ECEF  = 0x0101,
   MSG_POS_LLH   = 0x0102,
   MSG_TIME_UTC  = 0x0121,
   MSG_CFG_PRT   = 0x0600,
   MSG_CFG_MSG   = 0x0601,
   MSG_RXM_RAW   = 0x0210,
   MSG_UPD_DOWNL = 0x0901,
   MSG_INVALID   = 0xFFFF,
};

//MSG_RXM_SFRB   = 0x0211,
//MSG_UPD_UPLOAD = 0x0902,
//MSG_UPD_EXEC   = 0x0903,
//MSG_UPD_MEMCPY = 0x0904,
