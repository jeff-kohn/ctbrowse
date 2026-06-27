#pragma once

#include "App.h"
#include "model/DatasetWindow.h"

#include <wx/textctrl.h>

namespace ctb::app
{

   /// @brief base class used for our property value controls.
   ///
   /// This control displays a formatted single-line value in a read-only text control that looks
   /// like a static text control but supports selecting/copying text. The control will dynamically
   /// expand its width to accommodate changing values just like a static text ctrl.
   ///
   class ElasticPropertyValueBase : public DatasetWindow<wxTextCtrl>
   {
   public:
      auto hasDisplayValue() const -> bool
      {
         return m_display_value.empty() == false;
      }

   protected:
      ElasticPropertyValueBase(const DatasetEventSourcePtr& event_source);

      /// @brief retrieve the value to be displayed in the control from the dataset
      ///
      /// derived classes must implement this so that the control knows what to display
      ///
      /// @return formatted display string for the property value
      virtual auto getDisplayValue(const IDataset* ds) const -> std::string = 0;

   private:
      wxString m_display_value{};

      DECLARE_DATASET_WINDOW_FACTORY;

      void createWindow(wxWindow* parent) override;
      void onDatasetRowSelected(const DatasetEvent& event);
   };


}   // namespace ctb::app
