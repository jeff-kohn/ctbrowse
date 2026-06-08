#include "ElasticPropertyValueBase.h"

#include <ctb/tables/detail/field_helpers.h>
#include <wx/valgen.h>

namespace ctb::app
{

   ElasticPropertyValueBase::ElasticPropertyValueBase(const DatasetEventSourcePtr& event_source) : DatasetWindow<wxTextCtrl>(event_source)
   {
      SetValidator(wxGenericValidator{ &m_display_value });
   }


   void ElasticPropertyValueBase::onDatasetRowSelected(const DatasetEvent& event)
   {
      m_display_value = getDisplayValue(event.dataset);
      TransferDataToWindow();
      InvalidateBestSize();

      if (m_display_value.empty()) return;

      // a little wiggle room to ensure our new width doesn't cause horizontal scrolling when selecting text.
      constexpr auto select_margin = 3;

      // text controls don't auto-expand to fit text the way static controls do, have to force it.
      auto sz = GetSizeFromText(m_display_value);
      sz.SetWidth(sz.GetWidth() + select_margin);
      SetMinClientSize(sz);
   }


   void ElasticPropertyValueBase::createWindow(wxWindow* parent)
   {
      assert(parent);

      constexpr auto styles = wxTE_READONLY | wxBORDER_NONE;
      if (!wxTextCtrl::Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, styles))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }
      SetBackgroundColour(parent->GetBackgroundColour());
      SetValidator(wxGenericValidator{ &m_display_value });

      getEventHandler().addHandler(DatasetEventHandler::EventId::RowSelected, [this](const DatasetEvent& event) { onDatasetRowSelected(event); });
   }


}   // namespace ctb::app
