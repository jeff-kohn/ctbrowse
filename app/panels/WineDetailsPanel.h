/*********************************************************************
 * @file       WineDetailsPanel.h
 *
 * @brief      declaration for the WineDetailsPanel class
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/
#pragma once

#include "App.h"
#include "tasks.h"
#include "LabelImageCache.h"
#include "grid/ScopedEventSink.h"


#include <wx/panel.h>

// forward declarations for member ptrs
class wxGenericStaticBitmap;


namespace ctb::app
{

   class WineDetailsPanel final : public wxPanel, public IGridTableEventSink
   {
   public:

      /// @brief creates and initializes a panel for showing wine details
      ///
      /// throws a ctb::Error if parent or source = nullptr, or if the window can't be created;
      /// otherwise returns a non-owning pointer to the window (parent window will manage 
      /// its own lifetime). 
      /// 
      [[nodiscard]] static WineDetailsPanel* create(wxWindow* parent, GridTableEventSourcePtr source, LabelCachePtr cache);


      // no copy/move/assign, this class is created on the heap.
      WineDetailsPanel(const WineDetailsPanel&) = delete;
      WineDetailsPanel(WineDetailsPanel&&) = delete;
      WineDetailsPanel& operator=(const WineDetailsPanel&) = delete;
      WineDetailsPanel& operator=(WineDetailsPanel&&) = delete;
      ~WineDetailsPanel() override = default;

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
      void displayLabel();

      /// @brief status of MaybeImageTask
      //enum class LabelStatus
      //{
      //   Retrieved,
      //   Pending,
      //   Missing
      //};
      ///// @brief displays
      ///// @return 
      //auto checkForLabel() const -> LabelStatus;

      /// event source related handlers
      void notify(GridTableEvent event) override;
      void updateDetails(GridTableEvent event);

      // windows event handlers
      void onLabelTimer(wxTimerEvent& event);
      void onViewWebPage(wxCommandEvent& event);

      // private ctor used by create()
      explicit WineDetailsPanel(GridTableEventSourcePtr source, LabelCachePtr cache);
   };

} // namespace ctb::app

