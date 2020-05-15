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
#include "Profile.h"
#include <crtdbg.h>


CProfile::CProfile()
{
   // NOTE: Programmer must call Init function prior to
   // use, likely from the Dialog's OnInitDialog.
   // This class abuses CString, and will always throw an assertion.
   // This makes it hard to run in debug mode, so put the output to the
   // debug console instead of stopping with a dialog on failures.
   _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
}

CProfile::~CProfile()
{
   // Only save to file if the profile was changed.
   if(mNeedsSave)
   {
      CFile file;

      // Save the profile array to the storage file.
      if(!file.Open(mStrDataFullPath, CFile::modeCreate | CFile::modeWrite))
      {
         AfxMessageBox("Data settings could not be saved at: "
                       + mStrDataFullPath, MB_ICONSTOP);
      }
      else
      {
         // Write XML header.
         CString str;
         str = "<?xml version=\"1.0\"?>\r\n<!-- This file contains"\
               " persistent data for the application -->\r\n<data>\r\n";
         file.Write(str, str.GetLength());

         int i;
         for(i = 0; i < TABLE_SIZE; ++i)
         {
            _WriteElement(file,
                          mProfileTable[i].strSection,
                          mProfileTable[i].strEntry,
                          mProfileTable[i].strValue);
         }

         // Write XML footer.
         str = "</data>\r\n";
         file.Write(str, str.GetLength());

         file.Close();
      }
   }
}

void CProfile::Init(CString strDataFileName, bool bUseAppData)
{
   mNeedsSave = false;
   mWasInitPerformed = false;
   mStrDataFileName = strDataFileName;

   // Only allow init once.
   if(mWasInitPerformed)
   {
      // It could mess up the filepath if it is allowed to be called twice.
      AfxMessageBox("Programming error! Init called a second time.");
      exit(0);
   }
   mWasInitPerformed = true;

   // Keep track whether the profile was changed.
   mNeedsSave = false;
   CFile file;

   if(bUseAppData)
   {
      // Adjust the name to place the profile in the user's %appdata% folder.
      mStrDataFileName = PrepareAppDataFilename(mStrDataFileName);
   }

   // Load the profile array from the storage file.
   if(!file.Open(mStrDataFileName, CFile::modeRead))
   {
      // File didn't open properly so just set the filename.
      // we will create a new file later at destruction time.
      mStrDataFullPath = file.GetFilePath();
   }
   else
   {
      // Open the existing file and read in the data.
      mStrDataFullPath = file.GetFilePath();

      for(int i = 0; i < TABLE_SIZE; ++i)
      {
         _ReadElement(file, mProfileTable[i].strSection, mProfileTable[i].strEntry, mProfileTable[i].strValue);
      }

      file.Close();
   }
}

void CProfile::ClearProfile()
{
   CFile file;

   // Delete the XML file.
   file.Remove(mStrDataFullPath);
}

UINT CProfile::GetProfileInt(CString strSection, CString strEntry, int nDefault)
{
   UINT nValue = nDefault;

   int i = 0;

   if((i = _FindEntry(strSection, strEntry)) != -1)
   {
      nValue = atoi(mProfileTable[i].strValue);
   }

   return nValue;
}

bool CProfile::WriteProfileInt(CString strSection, CString strEntry, int nValue)
{
   bool bResult = true;

   // Flag as 'dirty', needs to be saved to file.
   mNeedsSave = true;

   int i;
   CString strValue;
   strValue.Format("%d", nValue);

   if((i = _FindEntry(strSection, strEntry)) == -1)
   {
      // Not found, so add new entry.
      _AddEntry(strSection, strEntry, strValue);
   }
   else
   {
      // Update the found entry.
      mProfileTable[i].strSection = strSection;
      mProfileTable[i].strEntry = strEntry;
      mProfileTable[i].strValue = strValue;

      bResult = true;
   }

   return bResult;
}

CString CProfile::GetProfileStr(CString strSection, CString strEntry, CString strDefault)
{
   CString strValue = strDefault;

   int i = 0;

   if((i = _FindEntry(strSection, strEntry)) != -1)
   {
      strValue = mProfileTable[i].strValue;
   }

   return strValue;
}

bool CProfile::WriteProfileStr(CString strSection, CString strEntry, CString strValue)
{
   bool bResult = true;

   // Flag as 'dirty', needs to be saved to file.
   mNeedsSave = true;

   int i;
   if((i = _FindEntry(strSection, strEntry)) == -1)
   {
      // Not found, so add new entry.
      _AddEntry(strSection, strEntry, strValue);
   }
   else
   {
      // Update the found entry.
      mProfileTable[i].strSection = strSection;
      mProfileTable[i].strEntry = strEntry;
      mProfileTable[i].strValue = strValue;

      bResult = true;
   }

   return bResult;
}

CString CProfile::GetFileName() const
{
   return mStrDataFullPath;
}

// PRIVATE FUNCTIONS
CString CProfile::PrepareAppDataFilename(CString strFileName)
{
   CString strFullPathFilename;

   // Get the user's specific %appdata% path. This is an environment variable,
   // so the user can adjust this value in the OS if they desire. (Add local
   // environment var with same name to override the OS default if necessary.)

   // Form the path name.
   CString strAppDataPath = getenv("AppData");
   strAppDataPath += "\\nzmCoder";

   // This will create our folder in AppData only if it does not exist.
   if(CreateDirectory(strAppDataPath, NULL) ||
         ERROR_ALREADY_EXISTS == GetLastError())
   {
      // One way or another, our AppData data folder now exists.
      // Concatenate to form the fully qualified user filename.
      strFullPathFilename = strAppDataPath + "\\" + strFileName;
   }
   else
   {
      //AfxMessageBox("Error using the AppData folder", MB_ICONERROR);

      // Error using AppData folder, so set the filename to the passed-in
      // value. This will safely fall back to making the file live in the
      // current folder, which is set to the same folder as the EXE.
      strFullPathFilename = strFileName;
   }

   // Return the full path and filename.
   return strFullPathFilename;
}

int CProfile::_FindEntry(CString strSection, CString strEntry)
{
   // Return index of found entry, -1 if not found.
   int nFoundIdx = -1;

   for(int i = 0; i < TABLE_SIZE; ++i)
   {
      if(mProfileTable[i].strSection == strSection
            && mProfileTable[i].strEntry == strEntry)
      {
         nFoundIdx = i;
         break;
      }
   }

   return nFoundIdx;
}

bool CProfile::_AddEntry(CString strSection, CString strEntry, CString strValue)
{
   bool bResult = false;

   for(int i = 0; i < TABLE_SIZE; ++i)
   {
      if(mProfileTable[i].strSection.IsEmpty())
      {
         // copy the strings in
         mProfileTable[i].strSection = strSection;
         mProfileTable[i].strEntry = strEntry;
         mProfileTable[i].strValue = strValue;

         bResult = true;
         break;
      }
   }

   return bResult;
}

void CProfile::_ReadElement(CFile& file, CString& strSection, CString& strEntry, CString& strValue)
{
   // assume valid until an error is found
   bool bValid = true;

   CString str;
   char c;
   int n;
   int i, j;

   // read one full element line from the file
   do
   {
      n = file.Read(&c, 1);
      str += c;
   }
   while(c != '/' && n == 1);

   if(n != 1) bValid = false;

   // Parse the "section" tag.
   n = str.Find("section=", 0);

   // Find the opening quote.
   i = str.Find("\"", n);

   // Find the closing quote.
   j = str.Find("\"", i + 1);

   // Found the full string.
   strSection = str.Mid(i + 1, j - i - 1);


   // Parse the "entry" tag.
   n = str.Find("entry=", j);

   // Find the opening quote.
   i = str.Find("\"", n);

   // Find the closing quote.
   j = str.Find("\"", i + 1);

   // Found the full string.
   strEntry = str.Mid(i + 1, j - i - 1);


   // Parse the "value" tag.
   n = str.Find("value=", j);

   // Find the opening quote.
   i = str.Find("\"", n);

   // Find the closing quote.
   j = str.Find("\"", i + 1);

   // Found the full string.
   strValue = str.Mid(i + 1, j - i - 1);
}

void CProfile::_WriteElement(CFile& file, CString strSection, CString strEntry, CString strValue)
{
   CString str;
   str.Format("<element section=\"%s\" entry=\"%s\" value=\"%s\"></element>\r\n",
              strSection,
              strEntry,
              strValue);

   file.Write(str, str.GetLength());
}

