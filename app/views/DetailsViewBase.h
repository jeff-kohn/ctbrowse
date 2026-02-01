/*********************************************************************
 * @file       DetailsViewBase.h
 *
 * @brief      declaration for the DetailsViewBase class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "App.h"
#include "LabelImageCache.h"

#include <ctb/model/DatasetEventHandler.h>

#include <wx/panel.h>

#include <map>


 // forward declaration for member ptr
class wxBoxSizer;

namespace ctb::app
{

   class DetailsViewBase : public wxPanel
   {
   public:
      // no copy/move/assign, this class is created on the heap and shouldn't be copied.
      DetailsViewBase(const DetailsViewBase&) = delete;
      DetailsViewBase(DetailsViewBase&&) = delete;
      DetailsViewBase& operator=(const DetailsViewBase&) = delete;
      DetailsViewBase& operator=(DetailsViewBase&&) = delete;
      ~DetailsViewBase() override = default;

      // Handles the window creation, since constructors only create the C++ object not the actual window. 
      virtual void createWindow(wxWindow* parent);

   protected:
      // this class can only be constructructed through derived classes
      DetailsViewBase(const DatasetEventSourcePtr& source) : m_dataset_events{ source }
      {}

      // Can be called by derived classes to add a commandlink button the to the specified sizer.
      void addCommandLinkButton(wxBoxSizer* sizer, CmdId cmd);
      
      // derived classes must override this to create their data-specific controls.
      virtual auto addDatasetSpecificControls(wxBoxSizer* top_sizer, const DatasetEventSourcePtr& source) -> void = 0;

   private:
      using wxPanel::Create;

      DatasetEventHandler    m_dataset_events;  
      wxString               m_drink_window_label{ constants::LBL_DRINK_WINDOW };

      void onCommand(wxCommandEvent& event);
   };


   // helper function for DetailsViewBase-derived classes that want to use DetailsViewBase::createWindow to handle window creation and 
   // just override DetailsViewBase::addDatasetSpecificControls to provide their customizations. If derived contructor is private
   // (as it should be to prevent stack-based instances), you'll need to delcare this function a friend to use it.
   template<typename ViewT>
   auto createDetailsViewFactory(wxWindow* parent, const DatasetEventSourcePtr& source) -> ViewT*
   {
      if (!parent)
      {
         assert("parent parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }
      if (!source)
      {
         assert("source parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }

      std::unique_ptr<ViewT> wnd{ new ViewT{ source } };
      wnd->createWindow(parent);
      return wnd.release();
   }


} // namespace ctb::app

