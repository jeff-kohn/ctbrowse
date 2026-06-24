#include "WineDetailTastingPanel.h"
#include "controls/ElasticMultiLineTextCtrl.h"

#include <wx/sizer.h>
#include <wx/valgen.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/wupdlock.h>

namespace ctb::app
{

   static inline wxString getTastingTitle(const IDataset* dataset)
   {
      if (dataset->getProperty(CtProp::TastingFlawed).asBool().value_or(false) == true)
      {
         return constants::STR_FLAWED_WINE;
      }
      auto maybe_liked = dataset->getProperty(CtProp::TastingLiked).asBool();
      if (maybe_liked.has_value())
      {
         return ctb::format(constants::FMT_TASTING_LIKE_MSG, *maybe_liked ? constants::STR_LIKE : constants::STR_DONT_LIKE);
      }
      return constants::LBL_TASTING_NOTE;
   }


   static inline wxString getTastingFeedbackText(const IDataset* dataset)
   {
      auto comments = dataset->getProperty(CtProp::TastingCommentCount).asInt32().value_or(0);
      auto views    = dataset->getProperty(CtProp::TastingViewCount).asInt32().value_or(0);
      auto votes    = dataset->getProperty(CtProp::TastingVoteCount).asInt32().value_or(0);

      if (votes and comments)
         return ctb::format(constants::FMT_TASTING_FEEDBACK_VWS_COMMENTS_VOTES, views, comments, votes);

      else if (votes)
         return ctb::format(constants::FMT_TASTING_FEEDBACK_VWS_VOTES, views, votes);

      else if (comments)
         return ctb::format(constants::FMT_TASTING_FEEDBACK_VWS_COMMENTS, views, comments);

      else
         return ctb::format(constants::FMT_TASTING_FEEDBACK_VIEWS, views);
   }


   //static inline wxString getTastingCtLikesText(const DatasetPtr& dataset, int rec_idx)
   //{
   //   auto like_pct = dataset->getProperty(rec_idx, CtProp::TastingCtLikePercent).asDouble().value_or(0.0) * 100; // convert to actual percent.
   //   auto likes    = dataset->getProperty(rec_idx, CtProp::TastingCtLikeCount).asInt32().value_or(0);
   //   return ctb::format(constants::FMT_TASTING_CT_LIKE_SUMMARY, likes, like_pct);
   //}


   void WineDetailTastingPanel::addDetails([[maybe_unused]] DetailRows& rows, const DatasetEventSourcePtr& source)
   {
      auto* top_sizer = GetSizer();  assert(top_sizer);

      // note title
      auto* title_ctrl = new wxStaticText(this, wxID_ANY, constants::LBL_TASTING_NOTE, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
      title_ctrl->SetValidator(wxGenericValidator{ &m_title });
      auto title_font = GetFont().MakeBold();
      title_ctrl->SetFont(title_font);
      title_ctrl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT));
      top_sizer->Add(title_ctrl, wxSizerFlags{}.Expand().Border(wxTOP | wxLEFT | wxRIGHT));

      // feedback summary
      auto* feedback_summary_ctrl = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
      feedback_summary_ctrl->SetValidator(wxGenericValidator{ &m_feedback_summary });
      feedback_summary_ctrl->SetFont(feedback_summary_ctrl->GetFont().MakeItalic());
      top_sizer->Add(feedback_summary_ctrl, wxSizerFlags{}.Center().Border(wxLEFT | wxRIGHT));

      // tasting note
      auto* note_ctrl = ElasticMultiLineTextCtrl::create(this, source, CtProp::TastingNotes);
      top_sizer->Add(note_ctrl, wxSizerFlags{ 2 }.Expand().TripleBorder());
   }


   void WineDetailTastingPanel::onDatasetEvent(const DatasetEvent& event)
   {
      if (event.dataset and event.affected_row.has_value())
      {
         m_title            = getTastingTitle(event.dataset);
         m_feedback_summary = getTastingFeedbackText(event.dataset);

         GetSizer()->ShowItems(true);
         Show(true);
      }
      else
      {
         GetSizer()->ShowItems(false);
         Show(false);
      }
      TransferDataToWindow();
   }

}   // namespace ctb::app
