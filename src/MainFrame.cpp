
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
