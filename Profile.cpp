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

// XML strings.
const CString XML_DIR_NAME = "nzmCoder";
const CString XML_COMMENT = "This file contains persistent data for the application";

// XML tag elements.
const char* XML_ROOT = "data";
const char* XML_ELEMENT = "element";
const char* XML_SECTION = "section";
const char* XML_ENTRY = "entry";
const char* XML_VALUE = "value";

CProfile::CProfile() :
   mXmlDocPtr(0),
   mIsInitialized(false)
{
   // Init function must be called by OnInitDialog.
}

CProfile::~CProfile()
{
   delete mXmlDocPtr;
   mXmlDocPtr = 0;
}

void CProfile::Init(CString strDataFilename, bool useAppDataLoc)
{
   // Only allow initialization once.
   if (!mIsInitialized)
   {
      mIsInitialized = true;
      mStrDataFilename = strDataFilename;

      if (useAppDataLoc)
      {
         // Adjust the name to place the profile in the user's APPDATA folder.
         mStrDataFilename = PrepareAppDataFilename(mStrDataFilename);
      }

      mXmlDocPtr = new tinyxml2::XMLDocument();

      if (mXmlDocPtr->LoadFile(mStrDataFilename) != tinyxml2::XML_SUCCESS)
      {
         ClearProfile();
      }
   }
}

void CProfile::ClearProfile()
{
   mXmlDocPtr->Clear();

   // Add the default declaration.
   tinyxml2::XMLDeclaration* pDeclaration = mXmlDocPtr->NewDeclaration(0);
   mXmlDocPtr->InsertEndChild(pDeclaration);
   mXmlDocPtr->SetBOM(true);

   // Add the header comment.
   tinyxml2::XMLComment* pComment = mXmlDocPtr->NewComment(XML_COMMENT);
   mXmlDocPtr->InsertEndChild(pComment);

   // Add the root element.
   tinyxml2::XMLElement* pRoot = mXmlDocPtr->NewElement(XML_ROOT);
   mXmlDocPtr->InsertEndChild(pRoot);

   // Save file.
   mXmlDocPtr->SaveFile(mStrDataFilename);
}

int CProfile::GetProfileInt(CString strSection, CString strEntry, int nDefault)
{
   int value = nDefault;
   bool isFound = false;

   tinyxml2::XMLElement* pRoot = mXmlDocPtr->FirstChildElement(XML_ROOT);
   if (pRoot)
   {
      tinyxml2::XMLElement* elementPtr = pRoot->FirstChildElement(XML_ELEMENT);
      while (elementPtr)
      {
         CString strXmlSection(elementPtr->Attribute(XML_SECTION));
         CString strXmlEntry(elementPtr->Attribute(XML_ENTRY));

         if (strXmlSection == strSection && strXmlEntry == strEntry)
         {
            CString strXmlValue(elementPtr->Attribute(XML_VALUE));

            value = atoi(strXmlValue);
            isFound = true;
            break;
         }

         elementPtr = elementPtr->NextSiblingElement(XML_ELEMENT);
      }
   }

   if (!isFound)
   {
      WriteProfileInt(strSection, strEntry, nDefault);
   }

   return value;
}

bool CProfile::WriteProfileInt(CString strSection, CString strEntry, int nValue)
{
   // Return value. True means attribute already existed.
   bool isFound = false;

   tinyxml2::XMLElement* pRoot = mXmlDocPtr->FirstChildElement(XML_ROOT);
   if (pRoot)
   {
      tinyxml2::XMLElement* elementPtr = pRoot->FirstChildElement(XML_ELEMENT);
      while (elementPtr)
      {
         CString strXmlSection(elementPtr->Attribute(XML_SECTION));
         CString strXmlEntry(elementPtr->Attribute(XML_ENTRY));

         if (strXmlSection == strSection && strXmlEntry == strEntry)
         {
            // Existing attribute, update it.
            elementPtr->SetAttribute(XML_VALUE, nValue);
            mXmlDocPtr->SaveFile(mStrDataFilename);

            isFound = true;
            break;
         }

         elementPtr = elementPtr->NextSiblingElement(XML_ELEMENT);
      }

      if (!isFound)
      {
         // A new attribute, add it.
         tinyxml2::XMLElement* pNewElement = mXmlDocPtr->NewElement(XML_ELEMENT);
         pNewElement->SetAttribute(XML_SECTION, strSection);
         pNewElement->SetAttribute(XML_ENTRY, strEntry);
         pNewElement->SetAttribute(XML_VALUE, nValue);
         pRoot->InsertEndChild(pNewElement);

         mXmlDocPtr->SaveFile(mStrDataFilename);
      }
   }

   return isFound;
}

CString CProfile::GetProfileStr(CString strSection, CString strEntry, CString strDefault)
{
   CString value = strDefault;
   bool isFound = false;

   tinyxml2::XMLElement* pRoot = mXmlDocPtr->FirstChildElement(XML_ROOT);
   if (pRoot)
   {
      tinyxml2::XMLElement* elementPtr = pRoot->FirstChildElement(XML_ELEMENT);
      while (elementPtr)
      {
         CString strXmlSection(elementPtr->Attribute(XML_SECTION));
         CString strXmlEntry(elementPtr->Attribute(XML_ENTRY));

         if (strXmlSection == strSection && strXmlEntry == strEntry)
         {
            CString strXmlValue(elementPtr->Attribute(XML_VALUE));

            value = strXmlValue;
            isFound = true;
            break;
         }

         elementPtr = elementPtr->NextSiblingElement(XML_ELEMENT);
      }
   }

   if (!isFound)
   {
      WriteProfileStr(strSection, strEntry, strDefault);
   }

   return value;
}

bool CProfile::WriteProfileStr(CString strSection, CString strEntry, CString strValue)
{
   // Return value. True means attribute already existed.
   bool isFound = false;

   tinyxml2::XMLElement* pRoot = mXmlDocPtr->FirstChildElement(XML_ROOT);
   if (pRoot)
   {
      tinyxml2::XMLElement* elementPtr = pRoot->FirstChildElement(XML_ELEMENT);
      while (elementPtr)
      {
         CString strXmlSection(elementPtr->Attribute(XML_SECTION));
         CString strXmlEntry(elementPtr->Attribute(XML_ENTRY));

         if (strXmlSection == strSection && strXmlEntry == strEntry)
         {
            // Existing attribute, update it.
            elementPtr->SetAttribute(XML_VALUE, strValue);
            mXmlDocPtr->SaveFile(mStrDataFilename);

            isFound = true;
            break;
         }

         elementPtr = elementPtr->NextSiblingElement(XML_ELEMENT);
      }

      if (!isFound)
      {
         // A new attribute, add it.
         tinyxml2::XMLElement* pNewElement = mXmlDocPtr->NewElement(XML_ELEMENT);
         pNewElement->SetAttribute(XML_SECTION, strSection);
         pNewElement->SetAttribute(XML_ENTRY, strEntry);
         pNewElement->SetAttribute(XML_VALUE, strValue);
         pRoot->InsertEndChild(pNewElement);

         mXmlDocPtr->SaveFile(mStrDataFilename);
      }
   }

   return isFound;
}

CString CProfile::PrepareAppDataFilename(CString strFileName)
{
   // Return value.
   CString strFullPathFilename;

   // Get the user's specific %appdata% path. This is an environment variable,
   // so the user can adjust this value in the OS if they desire. (Add local
   // environment var with same name to override the OS default if necessary.)

   // Form the path name.
   CString strAppDataPath = getenv("AppData");
   strAppDataPath += "\\" + XML_DIR_NAME;

   // This will create a folder in AppData only if it does not exist.
   if (CreateDirectory(strAppDataPath, NULL) ||
      ERROR_ALREADY_EXISTS == GetLastError())
   {
      // One way or another, the folder now exists.
      // Concatenate to form the fully qualified user filename.
      strFullPathFilename = strAppDataPath + "\\" + strFileName;
   }
   else
   {
      // Error using AppData folder, so set the filename to the passed-in
      // value. This will safely fall-back to making the file live in the
      // current folder, which is set to the same folder as the EXE.
      strFullPathFilename = strFileName;
   }

   // Return the full path and filename.
   return strFullPathFilename;
}
