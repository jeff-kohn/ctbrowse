/*********************************************************************
 * @file       DetailsViewTaggedWine.h
 *
 * @brief      declaration for the DetailsViewTaggedWine class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "App.h"
#include "views/DetailsViewBase.h"
#include "controls/LabelImageCtrl.h"
#include "controls/WineDetailTagsPanel.h"

namespace ctb::app
{

   class DetailsViewTaggedWine final : protected DetailsViewBase
   {
   public:
      /// @brief creates and initializes a view for showing wine details
      ///
      /// throws a ctb::Error if parent or source = nullptr, or if the window can't be created;
      /// otherwise returns a non-owning pointer to the window (parent window will manage 
      /// its lifetime). 
      /// 
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> DetailsViewBase*
      {
         return createDetailsViewFactory<DetailsViewTaggedWine>(parent, source);
      }

   protected:
      // this class can only be constructructed through static create(), which uses createDetailsViewFactory to call protected ctor
      template<typename BaseT>
      friend auto createDetailsViewFactory(wxWindow* parent, const DatasetEventSourcePtr& source) -> BaseT*;

      DetailsViewTaggedWine(DatasetEventSourcePtr source) : DetailsViewBase{ std::move(source) }
      {
      }


      // derived classes must implement this to add their view-specific controls
      auto addDatasetSpecificControls(wxBoxSizer* top_sizer, const DatasetEventSourcePtr& source) -> void override
      {
         const     auto sizer_flags = wxSizerFlags{}.Expand().Border(wxLEFT | wxRIGHT);
         constexpr auto heading_spacer = 3;
         constexpr auto group_spacer = heading_spacer * 3;

         top_sizer->AddSpacer(group_spacer);
         top_sizer->Add(WineDetailTagsPanel::create(this, source), sizer_flags);
         top_sizer->AddSpacer(group_spacer);
         addCommandLinkButton(top_sizer, CmdId::CMD_ONLINE_WINE_DETAILS);
         top_sizer->AddSpacer(heading_spacer);
         top_sizer->Add(LabelImageCtrl::create(this, source), wxSizerFlags().CenterHorizontal().Expand().Shaped());
      }
   };

} // namespace ctb::app

