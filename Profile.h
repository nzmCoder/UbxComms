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

class CProfile
{
public:
   CProfile();
   virtual ~CProfile();

   void Init(CString strDataFileName, bool bUseAppData = false);
   void ClearProfile();

   bool WriteProfileStr(CString strSection, CString strEntry, CString strValue);
   CString GetProfileStr(CString strSection, CString strEntry, CString strDefault);

   bool WriteProfileInt(CString strSection, CString strEntry, int nValue);
   UINT GetProfileInt(CString strSection, CString strEntry, int nDefault);

   CString GetFileName() const;

private:

   typedef struct
   {
      CString strSection;
      CString strEntry;
      CString strValue;
   } t_ProfileEntry;

   bool _AddEntry(CString strSection, CString strEntry, CString strValue);
   int _FindEntry(CString strSection, CString strEntry);

   void _ReadElement(CFile& file, CString& strSection, CString& strEntry, CString& strValue);
   void _WriteElement(CFile& file, CString strSection, CString strEntry, CString strValue);

   CString PrepareAppDataFilename(CString strFileName);

   // Data members.

   // The maximum number of stored elements.
   enum { TABLE_SIZE = 16 };

   t_ProfileEntry mProfileTable[TABLE_SIZE];
   CString mStrDataFullPath;
   bool mNeedsSave;
   CString mStrDataFileName;
   bool mWasInitPerformed;
   int dummy;
};
