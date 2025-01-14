/*********************************************************************
 * @file       MainFrame.cpp
 *
 * @brief      Implementation for the class MainFrame
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/

#include "MainFrame.h"  
#include "TableSyncDialog.h"

namespace cts
{

   void MainFrame::onMenuPreferences(wxCommandEvent& event) 
   {
   }

   void MainFrame::onMenuQuit(wxCommandEvent& event) 
   {
   }

   void MainFrame::onMenuSyncData(wxCommandEvent& event) 
   {
      TableSyncDialog dlg(this);
      if (dlg.ShowModal() != wxID_OK)
         return;
   }


} // namespace cts
