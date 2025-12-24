#include "WineDetailTastingPanel.h"
#include "controls/WineDetailFields.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/wupdlock.h>

namespace ctb::app
{

   static inline wxString getTastingTitle(const DatasetPtr& dataset, int rec_idx)
   {
      if (dataset->getProperty(rec_idx, CtProp::TastingFlawed).asBool().value_or(false) == true)
      {
         return constants::STR_FLAWED_WINE;
      }
      auto maybe_liked = dataset->getProperty(rec_idx, CtProp::TastingLiked).asBool();
      if (maybe_liked.has_value())
      {
         return ctb::format(constants::FMT_TASTING_LIKE_MSG, *maybe_liked ? constants::STR_LIKE : constants::STR_DONT_LIKE );
      }
      return constants::LBL_TASTING_NOTE;
   }


   static inline wxString getTastingFeedbackText(const DatasetPtr& dataset, int rec_idx)
   {
      auto comments = dataset->getProperty(rec_idx, CtProp::TastingCommentCount).asInt32().value_or(0);
      auto views    = dataset->getProperty(rec_idx, CtProp::TastingViewCount).asInt32().value_or(0);
      auto votes    = dataset->getProperty(rec_idx, CtProp::TastingVoteCount).asInt32().value_or(0);

      if (votes and comments)
         return ctb::format(constants::FMT_TASTING_FEEDBACK_VWS_COMMENTS_VOTES, views, comments, votes);

      else if (votes)
         return ctb::format(constants::FMT_TASTING_FEEDBACK_VWS_VOTES, views, votes);

      else if (comments)
         return ctb::format(constants::FMT_TASTING_FEEDBACK_VWS_COMMENTS, views, comments);

      else
         return ctb::format(constants::FMT_TASTING_FEEDBACK_VIEWS, views);
   }


   static inline wxString getTastingCtLikesText(const DatasetPtr& dataset, int rec_idx)
   {
      auto like_pct = dataset->getProperty(rec_idx, CtProp::TastingCtLikePercent).asDouble().value_or(0.0) * 100; // convert to actual percent.
      auto likes    = dataset->getProperty(rec_idx, CtProp::TastingCtLikeCount).asInt32().value_or(0);
      return ctb::format(constants::FMT_TASTING_CT_LIKE_SUMMARY, likes, like_pct);
   }



   WineDetailTastingPanel::WineDetailTastingPanel(wxWindow* parent, DatasetEventSourcePtr event_source) :
      wxPanel{ parent },
      m_event_handler{ event_source }
   {
      init();
   }


   void WineDetailTastingPanel::init()
   {
      static constexpr auto COL_COUNT = 2;

      wxWindowUpdateLocker freeze_win(this);

      auto top_sizer = new wxBoxSizer{ wxVERTICAL };
      SetSizer(top_sizer);

      // note title
      auto* title_ctrl = new wxStaticText(this, wxID_ANY,  constants::LBL_TASTING_NOTE, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
      title_ctrl->SetValidator(wxGenericValidator{ &m_title });
      auto title_font = GetFont().MakeBold();
      title_font.SetPointSize(title_font.GetPointSize() + 1);
      title_ctrl->SetFont(title_font);
      title_ctrl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT));
      top_sizer->Add(title_ctrl, wxSizerFlags{}.Expand().Border(wxTOP|wxLEFT|wxRIGHT));

      // feedback summary
      auto* feedback_summary_ctrl = new wxStaticText(this, wxID_ANY,  "", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
      feedback_summary_ctrl->SetValidator(wxGenericValidator{ &m_feedback_summary });
      feedback_summary_ctrl->SetFont(feedback_summary_ctrl->GetFont().MakeItalic());
      top_sizer->Add(feedback_summary_ctrl, wxSizerFlags{}.Center().Border(wxLEFT|wxRIGHT));

      // tasting note
      m_tasting_notes_ctrl = new wxStaticText(this, wxID_ANY,  "");//, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
      m_tasting_notes_ctrl->SetValidator(wxGenericValidator{ &m_tasting_notes });
      top_sizer->Add(m_tasting_notes_ctrl, wxSizerFlags{ 1 }.Expand().TripleBorder());

      // need to know when to update (or hide) the panel
      m_event_handler.addHandler(DatasetEvent::Id::DatasetRemove, [this](const DatasetEvent& event) { onDatasetEvent(event); });
      m_event_handler.addHandler(DatasetEvent::Id::Filter,        [this](const DatasetEvent& event) { onDatasetEvent(event); });
      m_event_handler.addHandler(DatasetEvent::Id::RowSelected,   [this](const DatasetEvent& event) { onDatasetEvent(event); });

      // handle resize so children are laid out correctly when this panel is resized
      Bind(wxEVT_SIZE, &WineDetailTastingPanel::onSize, this);

      Fit();
   }


   void WineDetailTastingPanel::onDatasetEvent(const DatasetEvent& event)
   {
      const auto& dataset = event.dataset;
      if (dataset and dataset->hasProperty(CtProp::TastingNotes) and event.affected_row.has_value())
      {
         auto rec_idx = event.affected_row.value();

         m_title               = getTastingTitle(dataset, rec_idx);
         m_feedback_summary    = getTastingFeedbackText(dataset, rec_idx);
         m_tasting_notes       = wxFromSV(dataset->getProperty(rec_idx, CtProp::TastingNotes).asStringView());

         GetSizer()->ShowItems(true);
         Show(true);
      }
      else {
         GetSizer()->ShowItems(false);
         Show(false);
      }
      // force full UI update
      TransferDataToWindow();
      SendSizeEvent();
   }


   void WineDetailTastingPanel::onSize(wxSizeEvent& event)
   {
      // The fact that we have to jump through this many hoops to get a static text to expand
      // vertically for large strings is beyond lame, this sizer stuff is a PITA sometimes.

      auto text_size = m_tasting_notes_ctrl->GetTextExtent(m_tasting_notes);
      if (text_size.x > event.m_size.x)
      {
         // text too long for one line, we need to calculate how many lines high the control should be to wrap
         text_size.y *= (text_size.x / event.m_size.x) + 1;
         text_size.x = event.m_size.x;
      }

      // Resize wine title to exact needed dimensions, then lay everything else out and wrap the text.
      m_tasting_notes_ctrl->SetClientSize(text_size);
      Layout();
      m_tasting_notes_ctrl->Wrap(m_tasting_notes_ctrl->GetClientSize().GetWidth());
      Refresh();
      Update();

      // Preserve default processing (important for proper propagation to parent/layout).
      event.Skip();
   }

} // namespace ctb::app