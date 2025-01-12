
#pragma once

#include "generated/MainFrameBase.h"

namespace cts
{

   class MainFrame : public MainFrameBase
   {
   public:
      MainFrame() {}  // If you use this constructor, you must call Create(parent)
      MainFrame(wxWindow* parent) : MainFrameBase(parent) {}

   private:
      void onMenuPreferences(wxCommandEvent& event) override;
      void onMenuQuit(wxCommandEvent& event) override;
      void onMenuSyncData(wxCommandEvent& event) override;
   };


} // namespace cts

