#include "WineDetailScorePanel.h"
#include "controls/WineDetailFields.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/wupdlock.h>

namespace ctb::app
{

   auto WineDetailScorePanel::create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailScorePanel*
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

      std::unique_ptr<WineDetailScorePanel> wnd{ new WineDetailScorePanel{ source } };
      wnd->createWindow(parent);
      return wnd.release(); // if we get here parent owns it, so return non-owning*
   }

   void WineDetailScorePanel::createWindow(wxWindow* parent)
   {
      static constexpr auto COL_COUNT = 2;

      if (!Create(parent))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }

      wxWindowUpdateLocker freeze_win(this);

      // heading
      auto* heading_lbl = new wxStaticText(this, wxID_ANY,  constants::LBL_SCORES, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
      heading_lbl->SetFont(GetFont().MakeBold());
      heading_lbl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT));

      // top level sizer contains the heading and the detail fields.
      auto top_sizer = new wxBoxSizer{ wxVERTICAL };
      SetSizer(top_sizer);
      top_sizer->Add(heading_lbl, wxSizerFlags{ 1 }.Expand().Border(wxBOTTOM | wxTOP));

      // ordering matters here because it's the same as they'll be displayed
      m_fields.push_back( SinglePropDetailField{ top_sizer, CtProp::MyScore, constants::LBL_MY_SCORE }.setFormat(constants::FMT_NUMBER_DECIMAL).setNullDisplayValue(constants::NO_SCORE));
      m_fields.push_back( SinglePropDetailField{ top_sizer, CtProp::CtScore, constants::LBL_CT_SCORE }.setFormat(constants::FMT_NUMBER_DECIMAL).setNullDisplayValue(constants::NO_SCORE));

      // need to know when to update (or hide) the panel
      m_dataset_events.addHandler(DatasetEvent::Id::DatasetRemove, [this](const DatasetEvent& event) { onDatasetEvent(event); });
      m_dataset_events.addHandler(DatasetEvent::Id::Filter,        [this](const DatasetEvent& event) { onDatasetEvent(event); });
      m_dataset_events.addHandler(DatasetEvent::Id::RowSelected,   [this](const DatasetEvent& event) { onDatasetEvent(event); });
   }


   void WineDetailScorePanel::onDatasetEvent(const DatasetEvent& event)
   {
      // only show this panel if score property present
      if (event.dataset->hasProperty(CtProp::CtScore) and event.affected_row.has_value())
      {
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