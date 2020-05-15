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
#include "Util.h"

uint32_t Util::Reverse32(uint32_t Data)
{
   return ((Data & 0xFF000000) >> 24) |
          ((Data & 0x00FF0000) >> 8)  |
          ((Data & 0x0000FF00) << 8)  |
          ((Data & 0x000000FF) << 24);
}

uint16_t Util::Reverse16(uint16_t Data)
{
   return (((Data & 0x00FF) << 8) |
           ((Data & 0xFF00) >> 8));
}
