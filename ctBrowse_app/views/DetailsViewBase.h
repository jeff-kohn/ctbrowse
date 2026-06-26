/*********************************************************************
 * @file       DetailsViewBase.h
 *
 * @brief      declaration for the DetailsViewBase class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "App.h"
#include "model/DatasetWindow.h"

#include <wx/panel.h>
#include <wx/weakref.h>


// forward declaration for member ptr
class wxBoxSizer;

namespace ctb::app
{

   class DetailsViewBase : public DatasetWindow<wxPanel>
   {
   public:
      using Base = DatasetWindow<wxPanel>;

      static constexpr int DEFAULT_HEADING_SPACER = 3;
      static constexpr int DEFAULT_GROUP_SPACER   = 3 * DEFAULT_HEADING_SPACER;

   protected:
      // this class can only be constructed through derived classes
      DetailsViewBase(const DatasetEventSourcePtr& source) : Base{ source }
      {}

      // Handles the window creation, since constructors only create the C++ object not the actual window. This implementation
      // will call wxPanel::Create(), then add the top/main details panel before calling addDatasetSpecificControls(), which
      // derived classes can use to add additional panels/buttons/etc. But this can be overridden if a different approach is needed.
      void createWindow(wxWindow* parent) override;

      // Can be called by derived classes to add a commandlink button the to the specified sizer.
      void addCommandLinkButton(wxBoxSizer* sizer, CmdId cmd);

      // derived classes must override this to create their data-specific controls.
      virtual auto addDatasetSpecificControls(wxBoxSizer* top_sizer, const DatasetEventSourcePtr& source) -> void = 0;

   private:
      using wxPanel::Create;
      void onCommand(wxCommandEvent& event);
   };

}   // namespace ctb::app

