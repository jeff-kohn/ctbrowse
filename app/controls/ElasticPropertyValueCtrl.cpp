#include "ElasticPropertyValueCtrl.h"

#include <ctb/tables/detail/field_helpers.h>
#include <wx/valgen.h>

namespace ctb::app
{

   ElasticPropertyValueCtrl::ElasticPropertyValueCtrl(const DatasetEventSourcePtr& event_source) : DatasetWindow<wxTextCtrl>(event_source)
   {
      SetValidator(wxGenericValidator{ &m_display_value });
   }


   void ElasticPropertyValueCtrl::onDatasetRowSelected(const DatasetEvent& event)
   {
      m_display_value = getDisplayValue(event.dataset, event.affected_row.value_or(0));
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


   void ElasticPropertyValueCtrl::createWindow(wxWindow* parent)
   {
      assert(parent);

      constexpr auto styles = wxTE_MULTILINE | wxTE_BESTWRAP | wxTE_READONLY | wxTE_NO_VSCROLL | wxBORDER_NONE;
      if (!wxTextCtrl::Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, styles))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }
      SetBackgroundColour(parent->GetBackgroundColour());
      SetValidator(wxGenericValidator{ &m_display_value });

      getEventHandler().addHandler(DatasetEventHandler::EventId::RowSelected, [this](const DatasetEvent& event) { onDatasetRowSelected(event); });
   }


   auto PropertyValueCtrl::create(wxWindow* parent, const DatasetEventSourcePtr& source, CtProp bound_prop) -> PropertyValueCtrl*
   {
      return detail::createDatasetWindow<PropertyValueCtrl>(parent, source, bound_prop);
   }


   auto PropertyValueCtrl::getDisplayValue(const DatasetPtr& ds, int rec_idx) const -> std::string
   {
      return ds->getProperty(rec_idx, m_prop).asString(m_format_str);
   }


   auto DrinkWindowCtrl::create(wxWindow* parent, const DatasetEventSourcePtr& source, CtProp begin_prop, CtProp end_prop) -> DrinkWindowCtrl*
   {
      return detail::createDatasetWindow<DrinkWindowCtrl>(parent, source, begin_prop, end_prop);
   }


   auto DrinkWindowCtrl::getDisplayValue(const DatasetPtr& ds, int rec_idx) const -> std::string
   {
      std::string value{};

      if (ds->hasProperty(m_end_prop))
      {
         auto begin_dt = ds->getProperty(rec_idx, m_begin_prop);
         auto end_dt   = ds->getProperty(rec_idx, m_end_prop);
         value         = ctb::detail::getDrinkWindow(begin_dt, end_dt);
      }
      return value;
   }


   auto ProReviewsCacheCtrl::create(wxWindow* parent, const DatasetEventSourcePtr& source, CacheValueFn value_fn) -> ProReviewsCacheCtrl*
   {
      return detail::createDatasetWindow<ProReviewsCacheCtrl>(parent, source, value_fn);
   }

   auto ProReviewsCacheCtrl::getDisplayValue(const DatasetPtr& ds, int rec_idx) const -> std::string
   {
      std::string value{};

      auto cache = wxGetApp().getProReviewsCache();
      if (cache)
      {
         auto wine_id = ds->getProperty(rec_idx, CtProp::iWineId).asUInt64().value_or(0);
         value        = (*cache.*m_value_fn)(wine_id);
      }
      return value;
   }

}   // namespace ctb::app
