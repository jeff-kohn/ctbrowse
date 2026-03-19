#include "WineDetailBottleInfoPanel.h"
#include "controls/WineDetailFields.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/wupdlock.h>


namespace ctb::app
{
   auto WineDetailBottleInfoPanel::create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailBottleInfoPanel*
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

      std::unique_ptr<WineDetailBottleInfoPanel> wnd{ new WineDetailBottleInfoPanel{ source } };
      wnd->createWindow(parent);
      return wnd.release(); // if we get here parent owns it, so return non-owning*
   }


   void WineDetailBottleInfoPanel::createWindow(wxWindow* parent)
   {
      if (!Create(parent))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }

      wxWindowUpdateLocker freeze_win(this);
      auto dataset = m_dataset_events.getDataset(true); // throws if dataset is nullptr
      
      // heading
      auto* heading_lbl = new wxStaticText(this, wxID_ANY, m_title, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
      heading_lbl->SetFont(GetFont().MakeBold());
      heading_lbl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT));

      // top level sizer contains the heading and the property grid of detail fields.
      auto* top_sizer = new wxBoxSizer{ wxVERTICAL };
      SetSizer(top_sizer);
      top_sizer->Add(heading_lbl, wxSizerFlags{ 1 }.Expand().Border(wxBOTTOM | wxTOP));

      // ordering matters here because it's the same as they'll be displayed
      m_fields.emplace_back(SinglePropDetailField{ top_sizer, CtProp::Size,              constants::LBL_SIZE        });
      m_fields.emplace_back(SinglePropDetailField{ top_sizer, CtProp::Location,          constants::LBL_LOCATION    });
      m_fields.emplace_back(SinglePropDetailField{ top_sizer, CtProp::Bin,               constants::LBL_BIN });

      if (dataset->hasProperty(CtProp::PendingOrderDate))
      {
         m_fields.emplace_back(SinglePropDetailField{ top_sizer, CtProp::PendingOrderDate,  constants::LBL_PURCHASED   }.setFormat(constants::FMT_DATE_SHORT));
      }
      if (dataset->hasProperty(CtProp::PendingStoreName))
      {
         m_fields.emplace_back(SinglePropDetailField{ top_sizer, CtProp::PendingStoreName,  constants::LBL_FROM        });
      }
      if (dataset->hasProperty(CtProp::ConsumeDate))
      {
         m_fields.emplace_back(SinglePropDetailField{ top_sizer, CtProp::ConsumeDate,    constants::LBL_CONSUME_DATE });
      }
      if (dataset->hasProperty(CtProp::ConsumeReason))
      {
         m_fields.emplace_back(SinglePropDetailField{ top_sizer, CtProp::ConsumeReason,  constants::LBL_CONSUME_REASON });
      }
      if (dataset->hasProperty(CtProp::BottleNote))
      {
         m_fields.emplace_back(SinglePropDetailField{ top_sizer, CtProp::BottleNote, constants::LBL_BOTTLE_NOTE });
      }
      if (dataset->hasProperty(CtProp::ConsumeNote))
      {
         m_fields.emplace_back(SinglePropDetailField{ top_sizer, CtProp::ConsumeNote, constants::LBL_CONSUME_NOTE });
      }

      // need to know when to update (or hide) the panel
      m_dataset_events.addHandler(DatasetEvent::Id::DatasetRemove, [this](const DatasetEvent& event) { onDatasetEvent(event); });
      m_dataset_events.addHandler(DatasetEvent::Id::Filter,        [this](const DatasetEvent& event) { onDatasetEvent(event); });
      m_dataset_events.addHandler(DatasetEvent::Id::RowSelected,   [this](const DatasetEvent& event) { onDatasetEvent(event); });
   }


   void WineDetailBottleInfoPanel::onDatasetEvent(const DatasetEvent & event)
   {
      // only show this panel if a row was selected.
      if (event.affected_row.has_value())
      {
         rng::for_each(m_fields, [&event](auto&& fld) { fld.update(event.dataset, event.affected_row.value()); });
         GetSizer()->ShowItems(true);
         Show(true);
      }
      else {
         rng::for_each(m_fields, [](auto&& fld) { fld.clear(); });
         GetSizer()->ShowItems(false);
         Show(false);
      }
      // force full UI update
      TransferDataToWindow();
      Layout();
      Refresh();
      Update();
   }
}