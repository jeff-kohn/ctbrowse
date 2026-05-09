#pragma once

#include "App.h"
#include "controls/WineDetailBasePanel.h"


namespace ctb::app
{
   /// @brief A wxPanel-derived class that displays details about a wine, handling dataset events and rendering relevant fields.
   ///
   class WineDetailTagsPanel final : public WineDetailBasePanel
   {
   public:
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailTagsPanel*
      {
         return detail::createDatasetWindow<WineDetailTagsPanel>(parent, source);
      }

   private:
      wxString      m_tag_note{};
      wxStaticText* m_tag_note_ctrl{};

      // this class can only be constructed through static create(), which uses createDetailsViewFactory to call private ctor
      template<typename WndT, typename... Args>
      friend auto detail::createDatasetWindow(wxWindow* parent, const DatasetEventSourcePtr& source, Args&&... args)->WndT*;

      WineDetailTagsPanel(const DatasetEventSourcePtr& event_source) : WineDetailBasePanel{ event_source }
      {}

      void getDetailFields(DetailFields& fields) override
      {
         auto* top_sizer = GetSizer(); assert(top_sizer);

         fields.push_back(SinglePropertyDisplay{ top_sizer, CtProp::TagName,      constants::LBL_TAG_NAME });
         fields.push_back(SinglePropertyDisplay{ top_sizer, CtProp::TagMaxPrice,  constants::LBL_MAX_PRICE }.setFormat(constants::FMT_NUMBER_CURRENCY));
      }

      void postWindowCreate() override
      {
         auto* top_sizer = GetSizer(); assert(top_sizer);

         m_tag_note_ctrl = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
         m_tag_note_ctrl->SetValidator(wxGenericValidator{ &m_tag_note });

         top_sizer->Add(m_tag_note_ctrl, wxSizerFlags{ 1 }.Border().Expand());
      }

   };
}