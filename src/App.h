//---------------------------------------------------------------------------------------------------------------------
// App.h
//
// header for the App class
//
// Copyright (c) 2025 Jeff Kohn. All Right Reserved.
//---------------------------------------------------------------------------------------------------------------------

#pragma once

#include "MainFrame.h"
#include "cts/constants.h"

#include <wx/wx.h>
#include <wx/confbase.h>


namespace cts
{

   /// <summary>
   ///   app object for the OuraCharts applicatin.
   /// </summary>
   class App : public wxApp
   {
   public:
      App();

      bool OnInit() override;
      int OnExit() override;


      /// <summary>
      ///   Get the current config object. Calling this will throw an exception
      ///   instead of returning nullptr if there's no default config.
      /// </summary>
      wxConfigBase& getConfig() noexcept(false);
      const wxConfigBase& getConfig() const noexcept(false);

   private:
      MainFrame* m_main_frame{};
   };

}  // namespace cts


wxDECLARE_APP(cts::App);
