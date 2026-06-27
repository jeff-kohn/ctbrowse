/*********************************************************************
 * @file       DetailsViewReadyToDrink.h
 *
 * @brief      declaration for the DetailsViewReadyToDrink class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "App.h"
#include "views/DetailsViewBase.h"
#include "panels/WineDetailScorePanel.h"

namespace ctb::app
{

   class DetailsViewReadyToDrink final : public DetailsViewBase
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
         return detail::createDatasetWindow<DetailsViewReadyToDrink>(parent, source);
      }

   protected:
      DetailsViewReadyToDrink(DatasetEventSourcePtr source) : DetailsViewBase{ std::move(source) }
      {}

      DECLARE_DATASET_WINDOW_FACTORY;

      // derived classes must implement this to add their view-specific controls
      auto addDatasetSpecificControls(wxBoxSizer* top_sizer, const DatasetEventSourcePtr& source) -> void override
      {
         top_sizer->AddSpacer(DEFAULT_HEADING_SPACER);
         top_sizer->Add(WineDetailScorePanel::create(this, source), wxSizerFlags{}.Expand().Border(wxLEFT | wxRIGHT));
         top_sizer->AddSpacer(DEFAULT_GROUP_SPACER);
         addCommandLinkButton(top_sizer, CmdId::CMD_ONLINE_DRINK_REMOVE);
         top_sizer->AddSpacer(DEFAULT_HEADING_SPACER);
         top_sizer->Add(LabelImageCtrl::create(this, source), wxSizerFlags().CenterHorizontal().Expand().Shaped());
      }
   };

}   // namespace ctb::app

