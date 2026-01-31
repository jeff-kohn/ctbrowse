#include "WineDetailMainPanel.h"

#include "controls/WineDetailFields.h"

#include <wx/stattext.h>
#include <wx/valgen.h>
#include <wx/sizer.h>
#include <wx/wupdlock.h>


namespace ctb::app
{
   auto WineDetailMainPanel::create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailMainPanel*
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

      std::unique_ptr<WineDetailMainPanel> wnd{ new WineDetailMainPanel{ source } };
      wnd->createWindow(parent);
      return wnd.release(); // if we get here parent owns it, so return non-owning*
   }

   void WineDetailMainPanel::createWindow(wxWindow* parent)
   {
      static constexpr auto COL_COUNT = 2;

      if (!Create(parent))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }

      wxWindowUpdateLocker freeze_win(this);

      auto dataset = m_dataset_events.getDataset(true); // throws if dataset is nullptr

      // wine name/title
      m_wine_ctrl = new wxStaticText( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER );
      m_wine_ctrl->SetValidator(wxGenericValidator(&m_wine_title));
      m_wine_ctrl->SetFont(GetFont().MakeLarger().MakeBold());
      m_wine_ctrl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT));

      // Top Level Sizer contains the wine title and the property grid of detail fields.
      auto top_sizer = new wxBoxSizer{ wxVERTICAL };
      SetSizer(top_sizer);
      top_sizer->Add(m_wine_ctrl, wxSizerFlags{}.Center().Border());

      // ordering matters here because it's the same as they'll be displayed
      m_fields.emplace_back( SinglePropDetailField  { top_sizer, CtProp::Vintage,        constants::LBL_VINTAGE     });
      m_fields.emplace_back( SinglePropDetailField  { top_sizer, CtProp::Varietal,       constants::LBL_VARIETAL    });
      m_fields.emplace_back( SinglePropDetailField  { top_sizer, CtProp::Country,        constants::LBL_COUNTRY     });
      m_fields.emplace_back( SinglePropDetailField  { top_sizer, CtProp::Region,         constants::LBL_REGION      });
      m_fields.emplace_back( SinglePropDetailField  { top_sizer, CtProp::SubRegion,      constants::LBL_SUB_REGION  });
      m_fields.emplace_back( SinglePropDetailField  { top_sizer, CtProp::Appellation,    constants::LBL_APPELLATION });
      m_fields.emplace_back( SinglePropDetailField  { top_sizer, CtProp::Size,           constants::LBL_SIZE        });

      if (dataset->hasProperty(CtProp::CtBeginConsume))
      {
         m_fields.emplace_back( DrinkWindowDetailField{ top_sizer, CtProp::BeginConsume,   CtProp::EndConsume,   constants::LBL_DRINK_WINDOW_MY  });
         m_fields.emplace_back( DrinkWindowDetailField { top_sizer, CtProp::CtBeginConsume, CtProp::CtEndConsume, constants::LBL_DRINK_WINDOW_CT });

      }
      else {
         m_fields.emplace_back(DrinkWindowDetailField{ top_sizer, CtProp::BeginConsume,   CtProp::EndConsume,   constants::LBL_DRINK_WINDOW });
      }

      if (dataset->hasProperty(CtProp::Location))
      {
         m_fields.emplace_back( SinglePropDetailField  { top_sizer, CtProp::Location,       constants::LBL_LOCATION       });
      }
      if (dataset->hasProperty(CtProp::ConsumeDate))
      {
         m_fields.emplace_back( SinglePropDetailField  { top_sizer, CtProp::ConsumeDate,    constants::LBL_CONSUME_DATE   });
      }
      if (dataset->hasProperty(CtProp::ConsumeReason))
      {
         m_fields.emplace_back( SinglePropDetailField  { top_sizer, CtProp::ConsumeReason,  constants::LBL_CONSUME_REASON });
      }

      // need to know when to update (or hide) the detail panels
      m_dataset_events.addHandler(DatasetEvent::Id::DatasetRemove, [this](const DatasetEvent& event){ onDatasetEvent(event); });
      m_dataset_events.addHandler(DatasetEvent::Id::Filter,        [this](const DatasetEvent& event){ onDatasetEvent(event); });
      m_dataset_events.addHandler(DatasetEvent::Id::RowSelected,   [this](const DatasetEvent& event){ onDatasetEvent(event); });

      // handle resize so children are laid out correctly when this panel is resized
      Bind(wxEVT_SIZE, &WineDetailMainPanel::onSize, this);

      Fit();
   }


   void WineDetailMainPanel::onDatasetEvent(const DatasetEvent& event)
   {
      if (event.affected_row.has_value())
      {
         // refresh everything since something affecting the current row happened
         auto update_visitor = [&event](auto&& field)
            {
               field.update(event.dataset, event.affected_row.value_or(0));
            };

         rng::for_each(m_fields, [&update_visitor](DetailField& fld) { std::visit(update_visitor, fld); });
         m_wine_title = event.dataset->getProperty(event.affected_row.value(), CtProp::WineName).asString();
      }
      else {
         // clear and hide everything until next row-level event.
         auto clear_visitor = [](auto&& field)
            {
               field.clear();
            };

         rng::for_each(m_fields, [&clear_visitor] (DetailField& fld) { std::visit(clear_visitor, fld); });
         m_wine_title.clear();
      }

      TransferDataToWindow();
      SendSizeEvent(); // So we can wrap the title
      Layout();
   }


   void WineDetailMainPanel::onSize(wxSizeEvent& event)
   {
      // Need to figure out how many lines to make the wine title so we can wrap it and show the full name
      constexpr auto margin = 5;
      if (event.m_size.x > 0)
      {
         m_wine_ctrl->Wrap(event.m_size.GetWidth() - margin);
         wxSize best_size = m_wine_ctrl->GetBestSize();
         m_wine_ctrl->SetClientSize(best_size);
      }
      Layout();
      Refresh();
      Update();

      // Preserve default processing (important for proper propagation to parent/layout).
      event.Skip();
   }

}