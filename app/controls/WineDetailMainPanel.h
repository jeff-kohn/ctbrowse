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
      WineDetailMainPanel(wxWindow* parent, DatasetEventSourcePtr event_source);

   private:
      using DetailField  = std::variant<SinglePropDetailField, DrinkWindowDetailField>;
      using DetailFields = std::deque<DetailField>;

      DatasetEventHandler m_event_handler;
      DetailFields        m_fields{};
      wxString            m_wine_title{};
      wxStaticText*       m_wine_ctrl{};
      void init();
      void onDatasetEvent(const DatasetEvent& event);

      // size event handler
      void onSize(wxSizeEvent& event);
   };


}