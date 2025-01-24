/*********************************************************************
 * @file       MainFrame.h
 *
 * @brief      Declaration for the class MainFrame
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "generated/MainFrameBase.h"
#include "grids/CellarTrackerGrid.h"

#include <format>


namespace ctb
{
   class MainFrame : public MainFrameBase
   {
   public:
      /// @brief default constructor for 2-phase initialization, must call Create(parent)
      MainFrame();  


      /// @brief constructor for immediate intialization, calls Create(parent for you)
      MainFrame(wxWindow* parent);


      /// @brief create the window object
      ///
      /// this should only be directly called if this object was default-constructed
      bool Create(wxWindow* parent);


      /// @brief set status bar text using std::format() syntax
      template <typename... Args>
      constexpr void setStatusText(std::format_string<Args...> fmt_str, Args&&... args)
      {
         SetStatusText(std::format(fmt_str, std::forward<Args>(args)...));
      }

   private:
      void onMenuPreferences(wxCommandEvent&) override;
      void onMenuSyncData(wxCommandEvent&) override;
      void onMenuWineList(wxCommandEvent&) override;
      void onQuit(wxCommandEvent& );

      CellarTrackerGrid* m_grid{ nullptr };
   };


} // namespace ctb

