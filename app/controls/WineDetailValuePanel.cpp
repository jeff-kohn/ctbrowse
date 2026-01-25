#include "WineDetailValuePanel.h"
#include "controls/WineDetailFields.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/wupdlock.h>

namespace ctb::app
{

   WineDetailValuePanel::WineDetailValuePanel(wxWindow* parent, const DatasetEventSourcePtr& event_source) :
      wxPanel{ parent },
      m_event_handler{ event_source }
   {
      init();
   }


   void WineDetailValuePanel::init()
   {
      static constexpr auto COL_COUNT = 2;

      wxWindowUpdateLocker freeze_win(this);

      // heading
      auto* heading_lbl = new wxStaticText(this, wxID_ANY,  constants::LBL_VALUATION, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
      heading_lbl->SetFont(GetFont().MakeBold());
      heading_lbl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT));

      // top level sizer contains the heading and the property grid of detail fields.
      auto top_sizer = new wxBoxSizer{ wxVERTICAL };
      SetSizer(top_sizer);
      top_sizer->Add(heading_lbl, wxSizerFlags{ 1 }.Expand().Border(wxBOTTOM | wxTOP));

      // ordering matters here because it's the same as they'll be displayed
      m_fields.push_back( SinglePropDetailField{ top_sizer, CtProp::MyPrice,      constants::LBL_MY_PRICE      }.setFormat(constants::FMT_NUMBER_CURRENCY));
      m_fields.push_back( SinglePropDetailField{ top_sizer, CtProp::CtPrice,      constants::LBL_CT_PRICE      }.setFormat(constants::FMT_NUMBER_CURRENCY));
      m_fields.push_back( SinglePropDetailField{ top_sizer, CtProp::AuctionPrice, constants::LBL_AUCTION_PRICE }.setFormat(constants::FMT_NUMBER_CURRENCY));

      // need to know when to update (or hide) the panel
      m_event_handler.addHandler(DatasetEvent::Id::DatasetRemove, [this](const DatasetEvent& event) { onDatasetEvent(event); });
      m_event_handler.addHandler(DatasetEvent::Id::Filter,        [this](const DatasetEvent& event) { onDatasetEvent(event); });
      m_event_handler.addHandler(DatasetEvent::Id::RowSelected,   [this](const DatasetEvent& event) { onDatasetEvent(event); });
   }


   void WineDetailValuePanel::onDatasetEvent(const DatasetEvent& event)
   {
      // only show this panel if score property present
      if (event.dataset->hasProperty(CtProp::MyPrice) and event.affected_row.has_value())
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