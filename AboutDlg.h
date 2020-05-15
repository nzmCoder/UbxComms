#pragma once
#include "resource.h"

class CAboutDlg : public CDialog
{
public:
   CAboutDlg();

   enum { IDD = IDD_ABOUT };

   DECLARE_MESSAGE_MAP()

protected:
   virtual void DoDataExchange(CDataExchange* pDX);
   virtual BOOL OnInitDialog();

private:
   CStatic mStatDateTime;
};

