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
#include "Crc.h"


uint16_t Crc::Calc16(uint8_t *dataPtr, uint16_t dataLength)
{
   uint8_t chkA = 0;
   uint8_t chkB = 0;

   for(int i = 0; i < dataLength; ++i)
   {
      chkA += dataPtr[i];
      chkB += chkA;
   }

   uint16_t retVal = (chkA << 8) + chkB;

   return retVal;
}
