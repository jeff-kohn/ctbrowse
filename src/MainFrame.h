/*********************************************************************
 * @file       MainFrame.h
 *
 * @brief      Declaration for the class MainFrame
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
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

