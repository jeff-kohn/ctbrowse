/*********************************************************************
 * @file       DetailsPanel.h
 *
 * @brief      declaration for the DetailsPanel class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "App.h"
#include "LabelImageCache.h"
#include <ctb/model/ScopedEventSink.h>

#include <ctb/tasks/tasks.h>
#include <wx/panel.h>
#include <wx/timer.h>


// forward declaration for member ptr
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
      [[nodiscard]] static DetailsPanel* create(wxWindow* parent, DatasetEventSourcePtr source, LabelCachePtr cache);

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
      using wxImageTask = LabelImageCache::wxImageTask;
      using MaybeImageTask = std::optional<wxImageTask>;

      /// @brief struct that control validators will be bound to for displaying in the window
      ///
      struct WineDetails
      {
         uint64_t wine_id{};
         wxString wine_name{};
         wxString vintage{};
         wxString varietal{};
         wxString country{};
         wxString region{};
         wxString sub_region{};
         wxString appellation{};
         wxString drink_window{};
         wxString my_score{};
         wxString ct_score{};
         wxString my_price{};
         wxString community_price{};
         wxString auction_value{};
         MaybeImageTask image_result{};
      };

      WineDetails            m_details{};
      ScopedEventSink        m_event_sink;   // no default init
      LabelCachePtr          m_label_cache{};
      wxGenericStaticBitmap* m_label_image{};
      wxTimer                m_label_timer{};

      // window creation
      void initControls();
      void checkLabelResult();
      void displayLabel();

      /// event source related handlers
      void notify(DatasetEvent event) override;
      void updateDetails(DatasetEvent event);

      // windows event handlers
      void onLabelTimer(wxTimerEvent& event);
      void onViewWebPage(wxCommandEvent& event);

      // private ctor used by create()
      explicit DetailsPanel(DatasetEventSourcePtr source, LabelCachePtr cache);
   };

} // namespace ctb::app

