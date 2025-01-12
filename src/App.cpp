/*********************************************************************
 * @file       App.cpp
 *
 * @brief      Implementation for the App class 
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/

#include "App.h"
#include "cts/Error.h"

#include <wx/stdpaths.h>
#include <wx/fileconf.h>

#include <chrono>

namespace cts
{
   namespace
   {
      const wxSize g_default_frame_size{ 400, 600 }; // NOLINT(cert-err58-cpp) this ctor doesn't throw it's just not marked noexcept
   }


   App::App()
   {
      SetUseBestVisual(true);

      // Set up config object to use file even on windows (registry is yuck)
      wxStandardPaths::Get().SetFileLayout(wxStandardPaths::FileLayout::FileLayout_XDG);
      wxConfigBase::DontCreateOnDemand();
      auto cfg = std::make_unique<wxFileConfig>(constants::APP_NAME_SHORT, 
                                                wxEmptyString, 
                                                wxEmptyString,  
                                                wxEmptyString,  
                                                wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_SUBDIR);
      wxConfigBase::Set(cfg.release());
   } // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks) unfortunately no way around it with wxWindows


   bool App::OnInit()
   {
      if (!wxApp::OnInit())
         return false;

      ::wxInitAllImageHandlers();

      m_main_frame = new MainFrame{nullptr};
      SetTopWindow(m_main_frame);
      m_main_frame->Center();
      m_main_frame->Show();

      return true;
   } 


   int App::OnExit()
   {
#ifdef _DEBUG
         // to prevent the tzdb allocations from being reported as memory leaks
         std::chrono::get_tzdb_list().~tzdb_list();
#endif
      return wxApp::OnExit();
   }


   wxConfigBase& App::getConfig() noexcept(false)
   {
      auto *config = wxConfigBase::Get(false);
      if (nullptr == config)
         throw Error{ "No configuration object available" };

      return *config;
   }


   const wxConfigBase& App::getConfig() const noexcept(false)
   {
      auto *config = wxConfigBase::Get(false);
      if (nullptr == config)
         throw Error{ "No configuration object available" };

      return *config;
   }

}  // namespace cts


// this needs to be outside the namespace for Linux but not Windows, go figure
wxIMPLEMENT_APP(cts::App);
