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
   /// pass the title (if any) to ctor land implement getDetailRows() to add the property fields.
   ///
   class WineDetailBasePanel : public DatasetWindow<wxPanel>
   {
   public:
      // no public interface for now.

   protected:
      using Base       = DatasetWindow<wxPanel>;
      using DetailRows = std::deque<WineDetailPanelRow>;

      /// @brief derived classes must implement this to add the rows the panel will display to the DetailRows container.
      virtual void getDetailRows(DetailRows& rows, const DatasetEventSourcePtr& event_source) = 0;

      /// @brief derived classes can override this to bind message handlers or carry out any other setup that requires
      ///        a valid window object.
      ///
      /// This is called after createWindow() has done initial setup including calling getDetailRows().
      virtual void postWindowCreate()
      {}

      /// @brief protected ctor
      ///
      /// empty title will not be displayed.
      WineDetailBasePanel(const DatasetEventSourcePtr& event_source, std::string_view title = "") : Base{ event_source }, m_title{ wxFromSV(title) }
      {}

      /// @brief virtual event handler function for derived classes to handle dataset events.
      ///
      /// derived classes need not call base-class version, as the base class does its necessary event handling in a private
      /// implementation before calling this.
      virtual void onDatasetEvent(const DatasetEvent& event)
      {}

   private:
      DetailRows m_rows{};
      wxString   m_title{};

      DECLARE_DATASET_WINDOW_FACTORY;

      /// @brief this method creates the panel window and adds the detail row controls. It also gives derived classes
      ///        a chance to add their controls by calling postWindowCreate().
      /// @param parent 
      void createWindow(wxWindow* parent) override;

      /// this is the registered event handler. It calls onDatasetEvent() so that derived classes can also get notifed.
      void onDatasetEventPrivate(const DatasetEvent& event);

      /// @brief this gets called for dataset events and will update the visiblity of detail rows depending on whether they contain
      ///        data for the current record or not. (null properties are not displayed unless they have a null display value).
      void updateVisibility();

   };

}   // namespace ctb::app
