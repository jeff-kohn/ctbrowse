#pragma once

#include "App.h"
#include "model/DatasetWindow.h"

#include <ctb/model/DatasetEventHandler.h>
#include <ctb\model\DatasetMgrAsyncCallbacks.h>
#include <wx/generic/statbmpg.h>
#include <wx/timer.h>
#include <wx/weakref.h>

class wxSizer;

namespace ctb::app
{


   class LabelImageCtrl final : public DatasetWindow<wxGenericStaticBitmap>
   {
   public:
      using Base = DatasetWindow<wxGenericStaticBitmap>;

      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> LabelImageCtrl*;


      /// @brief Callback we'll use to process background image retrieval callbacks.
      ///        Needs to be public otherwise it wouldn't be copyable/movable.
      class FetchLabelCallback
      {
      public:
         FetchLabelCallback(const wxWeakRef<LabelImageCtrl>& notification_wnd) : m_wnd(notification_wnd)
         {}

         void operator()(ImageResult result) const;

      private:
         wxWeakRef<LabelImageCtrl> m_wnd{};
      };

   private:
      uint64_t m_current_wine_id{}; // when a label notification comes in we need to be able to check that it's for current record

      DECLARE_DATASET_WINDOW_FACTORY;
      friend class FetchLabelCallback;

      LabelImageCtrl(const DatasetEventSourcePtr& source);

      void createWindow(wxWindow* parent) override;
      void labelUpdate(uint64_t wine_id, wxImage& image_result);
      void fetchImage(const DatasetEvent& event);
   };


}   // namespace ctb::app

