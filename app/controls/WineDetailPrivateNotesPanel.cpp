#include "WineDetailPrivateNotesPanel.h"
#include "controls/WineDetailFields.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/wupdlock.h>

namespace ctb::app
{


   void WineDetailPrivateNotesPanel::postWindowCreate()
   {
      auto* top_sizer = GetSizer(); assert(top_sizer);

      // private note
      m_private_note_ctrl = new wxStaticText(this, wxID_ANY,  "");//, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
      m_private_note_ctrl->SetValidator(wxGenericValidator{ &m_private_note });
      top_sizer->Add(m_private_note_ctrl, wxSizerFlags{ 2 }.Expand().TripleBorder());

      // handle resize so children are laid out correctly when this panel is resized
      Bind(wxEVT_SIZE, &WineDetailPrivateNotesPanel::onSize, this);
   }


   void WineDetailPrivateNotesPanel::onDatasetEvent(const DatasetEvent& event)
   {
      const auto& dataset = event.dataset;
      if (dataset and event.affected_row.has_value())
      {
         auto rec_idx = event.affected_row.value();
         m_private_note = wxFromSV(dataset->getProperty(rec_idx, CtProp::PrivateNote).asStringView());

         GetSizer()->ShowItems(true);
         Show(true);
      }
      else {
         GetSizer()->ShowItems(false);
         Show(false);
      }
      TransferDataToWindow();
      calcNoteSize();         // expand to fit note contents, parent will resize to accomodate
   }


   void WineDetailPrivateNotesPanel::onSize(wxSizeEvent& event)
   {
      // reset the label to remove any existing word-wrap, then re-fit/re-wrap the tasting note control for the new size.
      m_private_note_ctrl->SetLabel(m_private_note);
      calcNoteSize();

      // continue with parent processing
      event.Skip(); 
      return;
   }


   void WineDetailPrivateNotesPanel::calcNoteSize()
   {
      if (m_private_note.empty())
      {
         m_private_note_ctrl->SetClientSize(m_private_note_ctrl->GetBestSize());
      }
      else
      {
         // calculate how wide our note control can be and still fit in panel, allowing for sizer borders.
         constexpr auto margin = 30;
         const auto max_width = GetClientSize().GetWidth() - margin;
         m_private_note_ctrl->Wrap(max_width);
      }
   }

} // namespace ctb::app