/*********************************************************************
 * @file       DetailsPanel.h
 *
 * @brief      declaration for the DetailsPanel class
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
class wxGenericStaticBitmap;

namespace ctb::app
{
   class DetailsPanel final : public wxPanel, public IDatasetEventSink
   {
   public:
      /// @brief creates and initializes a panel for showing wine details
      ///
      /// throws a ctb::Error if parent or source = nullptr, or if the window can't be created;
      /// otherwise returns a non-owning pointer to the window (parent window will manage 
      /// its own lifetime). 
      /// 
      [[nodiscard]] static 
      auto create(wxWindow* parent, DatasetEventSourcePtr source, LabelCachePtr cache) -> DetailsPanel*;

      /// @brief Indicates whether the details for a selected wine are currently displayed.
      /// @return true if a wine is displayed in details, false otherwise.
      /// 
      auto wineDetailsActive() const -> bool;

      // no copy/move/assign, this class is created on the heap.
      DetailsPanel(const DetailsPanel&) = delete;
      DetailsPanel(DetailsPanel&&) = delete;
      DetailsPanel& operator=(const DetailsPanel&) = delete;
      DetailsPanel& operator=(DetailsPanel&&) = delete;
      ~DetailsPanel() override = default;

   private:
      using wxImageTask    = LabelImageCache::wxImageTask;
      using MaybeImageTask = std::optional<wxImageTask>;

      // these control categories allow us to show/hide different controls based on context of current dataset
      enum class ControlCategory
      {
         Consumed,
         CtDrinkWindow,        // ReadyToDrink dataset has both 'My' and 'CT' windows
         DrinkWindow,
         LinkAcceptPending,
         LinkOpenWineDetails,
         LinkReadyToDrink,
         Location,
         Pending,
         Score,
         Size,
         TastingNotes,
         Valuation,
         WineDetails,
      };
      using CategorizedControls = CategorizedControls<ControlCategory>;


      /// @brief struct that control validators will be bound to for displaying in the window
      struct WineDetails
      {
         std::string wine_id{};            // used for building CT url, not displayed
         wxString wine_name{};
         wxString vintage{};
         wxString varietal{};
         wxString country{};
         wxString region{};
         wxString sub_region{};
         wxString appellation{};
         wxString drink_window{};
         wxString ct_drink_window{};
         wxString consume_date{};
         wxString consume_reason{};
         wxString location{};
         wxString size{};
         wxString my_score{};
         wxString ct_score{};
         wxString my_price{};
         wxString community_price{};
         wxString auction_value{};

         std::string pending_purchase_id{}; // used for building CT url, not displayed
         wxString pending_order_date{};
         wxString pending_delivery_date{};
         wxString pending_store_name{};
         wxString pending_order_number{};
         wxString pending_qty{};
         wxString pending_price{};

         MaybeImageTask image_result{};
      };

      CategorizedControls    m_category_controls{};
      WineDetails            m_details{};
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
      void configureControlsForDataset(DatasetPtr dataset);
      void updateDetails(DatasetEvent event);

      // event handlers
      void onCommand(wxCommandEvent& event);
      void onLabelTimer(wxTimerEvent& event);

      // private ctor used by create()
      explicit DetailsPanel(DatasetEventSourcePtr source, LabelCachePtr cache);
   };

} // namespace ctb::app

