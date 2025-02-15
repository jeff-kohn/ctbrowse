#pragma once
#include "App.h"
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
      [[nodiscard]] static WineDetailsPanel* create(wxWindow* parent, GridTableEventSourcePtr source);

      // no copy/move/assign, this class is created on the heap.
      WineDetailsPanel(const WineDetailsPanel&) = delete;
      WineDetailsPanel(WineDetailsPanel&&) = delete;
      WineDetailsPanel& operator=(const WineDetailsPanel&) = delete;
      WineDetailsPanel& operator=(WineDetailsPanel&&) = delete;
      ~WineDetailsPanel() override = default;

   protected:

      // member variables
      wxStaticText*           m_appellation_txt{};
      wxStaticText*           m_auction_price_txt{};
      wxStaticText*           m_country_region_txt{};
      wxStaticText*           m_ct_price_txt{};
      wxStaticText*           m_ct_score_txt{};
      wxFlexGridSizer*        m_details_sizer{};
      wxGenericHyperlinkCtrl* m_drink_window_link{};
      ScopedEventSink         m_event_sink;           // no default init
      wxStaticText*           m_my_price_txt{};
      wxStaticText*           m_my_score_txt{};
      wxCollapsiblePane*      m_score_pane{};
      wxCollapsiblePane*      m_value_pane{};
      wxGenericHyperlinkCtrl* m_wine_name_link{};

      // window creation
      void initControls();

      /// event source related handlers
      void notify(GridTableEvent event, GridTable* grid_table) override;

      // windows event handlers
      void onPaneOpenClose(wxCollapsiblePaneEvent& event);

      // private ctor used by create()
      WineDetailsPanel(GridTableEventSourcePtr source);
   };

} // namespace ctb::app

