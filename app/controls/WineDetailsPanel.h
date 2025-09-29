#pragma once

#include "App.h"
#include "controls/DetailFields.h"

#include <ctb/model/DatasetEventHandler.h>

#include <wx/panel.h>


namespace ctb::app
{

   class WineDetailsPanel final : public wxPanel
   {
   public:
      WineDetailsPanel(wxWindow& parent, DatasetEventSourcePtr event_source);

   private:
      using DetailField = std::variant<SinglePropDetailField, DrinkWindowDetailField>;
      using DetailFields = std::vector<DetailField>;

      DatasetEventHandler m_event_handler;
      
      
   };


}