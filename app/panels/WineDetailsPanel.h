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

#include <wx/collpane.h>
#include <wx/gdicmn.h>
#include <wx/hyperlink.h>
#include <wx/generic/hyperlink.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

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
      using MaybeImageTask = std::optional<LabelImageCache::wxImageTask>;

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

      LabelCachePtr   m_label_cache{};
      ScopedEventSink m_event_sink;   // no default init
      WineDetails     m_details{};

      // window creation
      void initControls();

      /// event source related handlers
      void notify(GridTableEvent event) override;

      // windows event handlers
      void UpdateDetails(GridTableEvent event);
      void onViewWebPage(wxCommandEvent& event);

      // private ctor used by create()
      explicit WineDetailsPanel(GridTableEventSourcePtr source, LabelCachePtr cache);
   };

} // namespace ctb::app

