/*********************************************************************
 * @file       DatasetDetailsView.h
 *
 * @brief      declaration for the DatasetDetailsView class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "App.h"
#include "CategorizedControls.h"
#include "LabelImageCache.h"
#include <ctb/model/ScopedEventSink.h>

#include <wx/panel.h>
#include <wx/timer.h>

#include <map>

// forward declaration for member ptr
class wxBoxSizer;
class wxGridSizer;
class wxGenericStaticBitmap;

namespace ctb::app
{
   //class WineDetailsMainPanel;

   class DatasetDetailsView final : public wxPanel, public IDatasetEventSink
   {
   public:
      /// @brief creates and initializes a panel for showing wine details
      ///
      /// throws a ctb::Error if parent or source = nullptr, or if the window can't be created;
      /// otherwise returns a non-owning pointer to the window (parent window will manage 
      /// its own lifetime). 
      /// 
      [[nodiscard]] static 
      auto create(wxWindow* parent, const DatasetEventSourcePtr& source, LabelCachePtr cache) -> DatasetDetailsView*;

      // no copy/move/assign, this class is created on the heap and shouldn't be copied.
      DatasetDetailsView(const DatasetDetailsView&) = delete;
      DatasetDetailsView(DatasetDetailsView&&) = delete;
      DatasetDetailsView& operator=(const DatasetDetailsView&) = delete;
      DatasetDetailsView& operator=(DatasetDetailsView&&) = delete;
      ~DatasetDetailsView() override = default;

   private:
      using wxImageTask    = LabelImageCache::wxImageTask;
      using MaybeImageTask = std::optional<wxImageTask>;

      // these control categories allow us to show/hide different controls based on context of current dataset
      enum class ControlCategory
      {
         BottleImage,
         Consumed,
         CtDrinkWindow,        // ReadyToDrink dataset has both 'My' and 'CT' windows
         DrinkWindow,
         LinkAcceptPending,
         LinkOpenWineDetails,
         LinkReadyToDrink,
         Location,
         MyPrice,
         Pending,
         Score,
         Size,
         TastingNote,
         Valuation,
         WineDetails,
      };
      using CategorizedControls = CategorizedControls<ControlCategory>;


      CategorizedControls    m_category_controls{};
      MaybeImageTask         m_image_result{};
      ScopedEventSink        m_event_sink;   // no default init
      LabelCachePtr          m_label_cache{};
      wxGenericStaticBitmap* m_label_image{};
      wxTimer                m_label_timer{};
      wxString               m_drink_window_label{ constants::LBL_DRINK_WINDOW };

      // impl
      void initControls();
      void addCommandLinkButton(wxBoxSizer* sizer, CmdId cmd, CategorizedControls::Category category, std::string_view command_text, std::string_view note = constants::DETAILS_CMD_LINK_NOTE);
      void checkLabelResult();
      void displayLabel();

      /// event source related 
      void notify(DatasetEvent event) override;
      void configureControlsForDataset(const DatasetPtr& dataset);
      void updateDetails(DatasetEvent event);

      // event handlers
      void onCommand(wxCommandEvent& event);
      void onLabelTimer(wxTimerEvent& event);

      // private ctor used by create()
      explicit DatasetDetailsView(DatasetEventSourcePtr source, LabelCachePtr cache);
   };

} // namespace ctb::app

