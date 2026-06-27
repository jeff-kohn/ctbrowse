
#pragma once

#include "App.h"
#include "model/DatasetWindow.h"

#include <ctb/model/DatasetEventHandler.h>
#include <ctb/tables/CtSchema.h>

#include <wx/textctrl.h>


namespace ctb::app
{
   // multi-line, read-only dataset-bound text control that automatically resizes to fit its text
   //
   class ElasticMultiLineTextCtrl : public DatasetWindow<wxTextCtrl>
   {
   public:
      using Base = DatasetWindow<wxTextCtrl>;

      [[nodiscard]]
      static auto create(wxWindow* parent, const DatasetEventSourcePtr& source, CtProp bound_prop, wxAlignment align = wxALIGN_LEFT) -> ElasticMultiLineTextCtrl*;

      ~ElasticMultiLineTextCtrl() noexcept override = default;

      ElasticMultiLineTextCtrl(ElasticMultiLineTextCtrl&&)                 = delete;
      ElasticMultiLineTextCtrl(const ElasticMultiLineTextCtrl&)            = delete;
      ElasticMultiLineTextCtrl& operator=(ElasticMultiLineTextCtrl&&)      = delete;
      ElasticMultiLineTextCtrl& operator=(const ElasticMultiLineTextCtrl&) = delete;

   private:
      wxString    m_display_value{};
      CtProp      m_prop;
      bool        m_need_resize{ false };
      wxAlignment m_align_flag{};

      ElasticMultiLineTextCtrl(const DatasetEventSourcePtr& source, CtProp bound_prop, wxAlignment align = wxALIGN_LEFT)
         : Base{ source },
           m_prop(bound_prop),
           m_align_flag{ align }
      {}

      DECLARE_DATASET_WINDOW_FACTORY;

      void createWindow(wxWindow* parent) override;
      auto calcTextSize() -> wxSize;

      void onDatasetRowSelected(const DatasetEvent& event);
      void onSize(wxSizeEvent& event);
   };

}   // namespace ctb::app
