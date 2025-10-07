#include "WineDetailMainPanel.h"

#include "controls/WineDetailFields.h"

#include <wx/stattext.h>
#include <wx/valgen.h>
#include <wx/sizer.h>
#include <wx/wupdlock.h>


namespace ctb::app
{
   WineDetailMainPanel::WineDetailMainPanel(wxWindow* parent, DatasetEventSourcePtr event_source) :
      wxPanel{ parent },
      m_event_handler { event_source }
   {
      init();
   }


   void WineDetailMainPanel::init()
   {
      static constexpr auto COL_COUNT = 2;

      wxWindowUpdateLocker freeze_win(this);

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
      m_fields.emplace_back( DrinkWindowDetailField { top_sizer, CtProp::BeginConsume,   CtProp::EndConsume,   constants::LBL_DRINK_WINDOW    });
      m_fields.emplace_back( DrinkWindowDetailField { top_sizer, CtProp::CtBeginConsume, CtProp::CtEndConsume, constants::LBL_DRINK_WINDOW_CT });
      m_fields.emplace_back( SinglePropDetailField  { top_sizer, CtProp::Location,       constants::LBL_LOCATION       });
      m_fields.emplace_back( SinglePropDetailField  { top_sizer, CtProp::ConsumeDate,    constants::LBL_CONSUME_DATE   });
      m_fields.emplace_back( SinglePropDetailField  { top_sizer, CtProp::ConsumeReason,  constants::LBL_CONSUME_REASON });

      // need to know when to update (or hide) the detail panels
      m_event_handler.addHandler(DatasetEvent::Id::DatasetRemove, [this](const DatasetEvent& event){ onDatasetEvent(event); });
      m_event_handler.addHandler(DatasetEvent::Id::Filter,        [this](const DatasetEvent& event){ onDatasetEvent(event); });
      m_event_handler.addHandler(DatasetEvent::Id::RowSelected,   [this](const DatasetEvent& event){ onDatasetEvent(event); });

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
   }


   void WineDetailMainPanel::onSize(wxSizeEvent& event)
   {
      // The fact that we have to jump through this many hoops to get a static text to expand
      // vertically for large strings is beyond lame, this sizer stuff is a PITA sometimes.

      auto text_size = m_wine_ctrl->GetTextExtent(m_wine_title);
      if (text_size.x > event.m_size.x)
      {
         // text too long for one line, we need to calculate how many lines high the control should be to wrap
         text_size.y *= (text_size.x / event.m_size.x);
         text_size.x = event.m_size.x;
      }

      // Resize wine title to exact needed dimensions, then lay everything else out and wrap the text.
      m_wine_ctrl->SetClientSize(text_size);
      Layout();
      m_wine_ctrl->Wrap(event.m_size.GetWidth());
      Refresh();
      Update();

      // Preserve default processing (important for proper propagation to parent/layout).
      event.Skip();
   }

}