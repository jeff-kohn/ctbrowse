#include "WineDetailTagsPanel.h"
#include "controls/WineDetailFields.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/wupdlock.h>

namespace ctb::app
{

   auto WineDetailTagsPanel::create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailTagsPanel*
   {
      if (!parent)
      {
         assert("parent window cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }
      if (!source)
      {
         assert("source parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }

      std::unique_ptr<WineDetailTagsPanel> wnd{ new WineDetailTagsPanel{ source } };
      wnd->createWindow(parent);
      return wnd.release(); // if we get here parent owns it, so return non-owning*
   }

   void WineDetailTagsPanel::createWindow(wxWindow* parent)
   {
      static constexpr auto COL_COUNT = 2;

      if (!Create(parent))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }

      wxWindowUpdateLocker freeze_win(this);

      auto top_sizer = new wxBoxSizer{ wxVERTICAL };
      SetSizer(top_sizer);

      m_fields.push_back(SinglePropDetailField{ top_sizer, CtProp::TagName,      constants::LBL_TAG_NAME });
      m_fields.push_back(SinglePropDetailField{ top_sizer, CtProp::TagMaxPrice,  constants::LBL_MAX_PRICE }.setFormat(constants::FMT_NUMBER_CURRENCY));

      m_tag_note_ctrl = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
      m_tag_note_ctrl->SetValidator(wxGenericValidator{ &m_tag_note });

      top_sizer->Add(m_tag_note_ctrl, wxSizerFlags{ 1 }.Border().Expand());

      // need to know when to update (or hide) the panel
      m_event_handler.addHandler(DatasetEvent::Id::DatasetRemove, [this](const DatasetEvent& event) { onDatasetEvent(event); });
      m_event_handler.addHandler(DatasetEvent::Id::Filter,        [this](const DatasetEvent& event) { onDatasetEvent(event); });
      m_event_handler.addHandler(DatasetEvent::Id::RowSelected,   [this](const DatasetEvent& event) { onDatasetEvent(event); });
   }


   void WineDetailTagsPanel::onDatasetEvent(const DatasetEvent& event)
   {
      // only show this panel if score property present
      if (event.dataset->hasProperty(CtProp::TagName) and event.affected_row.has_value())
      {
         m_tag_note = event.dataset->getProperty(event.affected_row.value(), CtProp::TagWineNote).asString();
         rng::for_each(m_fields, [&event](auto&& fld) { fld.update(event.dataset, event.affected_row.value()); });
         Show(true);
         GetSizer()->ShowItems(true);
      }
      else {
         rng::for_each(m_fields, [&event](auto&& fld) { fld.clear(); });
         Show(false);
         GetSizer()->ShowItems(false);
      }
      // force full UI update
      TransferDataToWindow();
      Layout();
      Refresh();
      Update();
   }

} // namespace ctb::app