#pragma once

#include "App.h"
#include "model/DatasetWindow.h"
#include "panels/WineDetailPanelRow.h"

#include <wx/panel.h>

namespace ctb::app
{

   /// @brief  base class used for deriving classes that implement a property panel
   ///         showing an optional title along with a 2-column table of property labels and values.
   ///
   /// To make use of this class, all the derived class has to do is inherit from WineDetailBasePanel
   /// pass the title (if any) to ctor and implement addDetails() to add the property fields.
   ///
   class WineDetailBasePanel : public DatasetWindow<wxPanel>
   {
   public:
      // no public interface for now.

   protected:
      using Base       = DatasetWindow<wxPanel>;
      using DetailRows = std::deque<WineDetailPanelRow>;

      /// @brief derived classes must implement this to add the detail rows the panel will display.
      ///
      /// This is called just after window creation. Other controls besides DetailRows can also be added,
      /// but won't have their visibility managed automatically. Message bindings can also be set up if needed.
      virtual void addDetails(DetailRows& rows, const DatasetEventSourcePtr& event_source) = 0;

      /// @brief protected ctor
      /// @param source - event source that controls should be bound to
      /// @param title - The title to use for the panel heading. If blank no heading will be shown.
      WineDetailBasePanel(const DatasetEventSourcePtr& source, std::string_view title = "") : Base{ source }, m_title{ wxFromSV(title) }
      {}

      /// @brief virtual event handler function for derived classes to handle dataset events.
      ///
      /// derived classes need not call the base-class version, since base class uses private handler 
      /// before calling this. Overriding this will often be unnecessary for panels that use bound
      /// controls which have their own event handlers.
      virtual void onDatasetEvent([[maybe_unused]] const DatasetEvent& event)
      {}

   private:
      DetailRows m_rows{};
      wxString   m_title{};

      DECLARE_DATASET_WINDOW_FACTORY;

      /// @brief this method creates the panel window and adds the detail rows. It also gives derived classes
      ///        a chance to add their controls.
      /// @param parent 
      void createWindow(wxWindow* parent) override;

      /// this is the registered event handler. It calls the protected onDatasetEvent() so that derived classes can also get notified.
      void onDatasetEventPrivate(const DatasetEvent& event);

      /// @brief this gets called for dataset events and will update the visibility of detail rows depending on whether they contain
      ///        data for the current record or not. (null properties are not displayed unless they have a null display value).
      void updateVisibility();
   };

}   // namespace ctb::app
