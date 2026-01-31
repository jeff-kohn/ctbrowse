#pragma once

#include "App.h"
#include "LabelImageCache.h"
#include <ctb/model/DatasetEventHandler.h>

#include <wx/generic/statbmpg.h>
#include <wx/timer.h>


class wxSizer;

namespace ctb::app
{

   class LabelImageCtrl final : public wxGenericStaticBitmap
   {
   public:
      static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> LabelImageCtrl*;

      // no copy/move/assign, this class is created on the heap and owned by parent window.
      LabelImageCtrl() = delete;
      LabelImageCtrl(const LabelImageCtrl&) = delete;
      LabelImageCtrl(LabelImageCtrl&&) = delete;
      LabelImageCtrl& operator=(const LabelImageCtrl&) = delete;
      LabelImageCtrl& operator=(LabelImageCtrl&&) = delete;
      ~LabelImageCtrl() noexcept override = default;

   private:
      LabelImageCtrl(const DatasetEventSourcePtr& source, LabelCachePtr cache);

      void createWindow(wxWindow* parent);

      using wxImageTask    = LabelImageCache::wxImageTask;
      using MaybeImageTask = std::optional<wxImageTask>;

      LabelCachePtr          m_cache{};
      DatasetEventHandler    m_event_handler;
      MaybeImageTask         m_image_result{};
      wxTimer                m_label_timer{};

      void checkLabelResult();
      void displayLabel();
      void onLabelTimer(wxTimerEvent& event);
      void fetchImage(const DatasetEvent& event);
   };


} // namespace ctb::app
