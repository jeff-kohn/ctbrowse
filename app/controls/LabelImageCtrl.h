#pragma once

#include "App.h"
#include "LabelImageCache.h"
#include "model/DatasetWindow.h"

#include <ctb/model/DatasetEventHandler.h>

#include <wx/generic/statbmpg.h>
#include <wx/timer.h>


class wxSizer;

namespace ctb::app
{

   class LabelImageCtrl final : public DatasetWindow<wxGenericStaticBitmap>
   {
   public:
      using Base = DatasetWindow<wxGenericStaticBitmap>;

      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> LabelImageCtrl*;

   private:
      using MaybeImageTask = std::optional<wxImageTask>;

      LabelCachePtr          m_cache{};
      MaybeImageTask         m_image_result{};
      wxTimer                m_label_timer{};

      // this class can only be constructructed through static create(), which uses createDetailsViewFactory to call protected ctor
      template<typename WndT, typename... Args>
      friend auto detail::createDatasetWindow(wxWindow* parent, const DatasetEventSourcePtr& source, Args... args)->WndT*;

      LabelImageCtrl(const DatasetEventSourcePtr& source, LabelCachePtr cache);

      void createWindow(wxWindow* parent) override;
      void checkLabelResult();
      void displayLabel();
      void onLabelTimer(wxTimerEvent& event);
      void fetchImage(const DatasetEvent& event);
   };


} // namespace ctb::app
