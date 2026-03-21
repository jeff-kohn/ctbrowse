#include "WineDetailBasePanel.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/wupdlock.h>

namespace ctb::app
{

   void WineDetailBasePanel::onDatasetEvent(const DatasetEvent& event)
   {
      if (event.affected_row.has_value())
      {
         // refresh everything since something affecting the current row happened
         auto update_visitor = [&event](auto&& field)
            {
               field.update(event.dataset, event.affected_row.value_or(0));
            };

         rng::for_each(m_fields, [&update_visitor](DetailField& fld) { std::visit(update_visitor, fld); });
      }
      else {
         // clear and hide everything until next row-level event.
         auto clear_visitor = [](auto&& field)
            {
               field.clear();
            };

         rng::for_each(m_fields, [&clear_visitor](DetailField& fld) { std::visit(clear_visitor, fld); });
      }

      TransferDataToWindow();
      SendSizeEvent(); // So we can wrap the title
      Layout();
   }


   void WineDetailBasePanel::createWindow(wxWindow* parent)
   {
      if (!Create(parent))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }

      wxWindowUpdateLocker freeze_win(this);

      // top level sizer contains the heading and the property grid of detail fields.
      auto* top_sizer = new wxBoxSizer{ wxVERTICAL };
      SetSizer(top_sizer);

      // heading
      if (!m_title.empty())
      {
         auto* heading_lbl = new wxStaticText(this, wxID_ANY, m_title, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
         heading_lbl->SetFont(GetFont().MakeBold());
         heading_lbl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT));
         top_sizer->Add(heading_lbl, wxSizerFlags{ 1 }.Expand().Border(wxBOTTOM | wxTOP));
      }

      getDetailFields(m_fields);
      postWindowCreate();
      Fit();

      getEventHandler().setDefaultHandler([this](const DatasetEvent& event) { onDatasetEvent(event); });
   }

} // namespace ctb::app