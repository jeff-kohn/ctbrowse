#pragma once

#include "App.h"
#include "controls/WineDetailFields.h"

#include <ctb/model/DatasetEventHandler.h>
#include <wx/panel.h>
#include <deque>


class wxStaticText;

namespace ctb::app
{
   
   /// @brief A wxPanel-derived class that displays details about a wine, handling dataset events and rendering relevant fields.
   ///
   class WineDetailMainPanel final : public wxPanel
   {
   public:
      static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailMainPanel*;

   private:
      using DetailField  = std::variant<SinglePropDetailField, DrinkWindowDetailField>;
      using DetailFields = std::deque<DetailField>;

      DatasetEventHandler m_event_handler;
      DetailFields        m_fields{};
      wxString            m_wine_title{};
      wxStaticText*       m_wine_ctrl{};

      WineDetailMainPanel(const DatasetEventSourcePtr& event_source) : m_event_handler{ event_source }
      {}

      void createWindow(wxWindow* parent);
      void onDatasetEvent(const DatasetEvent& event);

      // size event handler
      void onSize(wxSizeEvent& event);
   };


}