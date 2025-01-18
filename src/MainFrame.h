/*********************************************************************
 * @file       MainFrame.h
 *
 * @brief      Declaration for the class MainFrame
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "generated/MainFrameBase.h"
#include "CtGridTable.h"
#include "cts/data/DataManager.h"
#include "cts/data/WineList.h"
#include <format>


namespace cts
{
   class MainFrame : public MainFrameBase
   {
   public:
      MainFrame();  // If you use this constructor, you must call Create(parent)
      MainFrame(wxWindow* parent);

      /// @brief create the window object
      ///
      /// this should only be called if this object was default-constructed
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

      wxGrid* m_grid{};
      data::DataManager m_data_mgr;
      std::shared_ptr<data::CtGridTable> m_wine_list{};
   };


} // namespace cts

