#pragma once

#include "App.h"
#include "controls/WineDetailBasePanel.h"

class wxStaticText;


namespace ctb::app
{
   /// @brief A panel class that displays details about a wine, handling dataset events and rendering relevant fields.
   ///
   class WineDetailMainPanel final : public WineDetailBasePanel
   {
   public:
      static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> WineDetailMainPanel*;

   private:
      wxString      m_wine_title{};
      wxStaticText* m_wine_ctrl{};

      // this class can only be constructed through static create(), which uses createDetailsViewFactory to call private ctor
      template<typename WndT, typename... Args>
      friend auto detail::createDatasetWindow(wxWindow* parent, const DatasetEventSourcePtr& source, Args&&... args)->WndT*;

      WineDetailMainPanel(const DatasetEventSourcePtr& event_source) : WineDetailBasePanel{ event_source }
      {}

      // base class overrides
      void getDetailFields(DetailFields& fields) override;
      void postWindowCreate() override;
      void onDatasetEvent(const DatasetEvent& event) override;

      // size event handler for wrapping wine title.
      void onSize(wxSizeEvent& event);
   };



}