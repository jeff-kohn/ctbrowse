
#pragma once

#include "App.h"
#include "model/DatasetWindow.h"

#include <ctb/model/DatasetEventHandler.h>
#include <ctb/tables/CtSchema.h>

#include <wx/textctrl.h>


namespace ctb::app
{
   // multi-line, readonly text control that automatically resizes to fit its text
   //
   class ElasticMultiTextCtrl : public DatasetWindow<wxTextCtrl>
   {
   public:
      using Base = DatasetWindow<wxTextCtrl>;

      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source, CtProp bound_prop) -> ElasticMultiTextCtrl*;

      ~ElasticMultiTextCtrl() noexcept override = default;

      ElasticMultiTextCtrl(ElasticMultiTextCtrl&&)                 = delete;
      ElasticMultiTextCtrl(const ElasticMultiTextCtrl&)            = delete;
      ElasticMultiTextCtrl& operator=(ElasticMultiTextCtrl&&)      = delete;
      ElasticMultiTextCtrl& operator=(const ElasticMultiTextCtrl&) = delete;

   private:
      wxString m_display_value{};
      CtProp   m_prop;
      bool     m_need_resize{ false };

      ElasticMultiTextCtrl(const DatasetEventSourcePtr& source, CtProp bound_prop) : Base{ source }, m_prop(bound_prop)
      {}

      // this class can only be constructed through static create(), which uses createDetailsViewFactory to call private ctor
      template<typename WndT, typename... Args>
      friend auto detail::createDatasetWindow(wxWindow* parent, const DatasetEventSourcePtr& source, Args&&... args) -> WndT*;

      void createWindow(wxWindow* parent) override;
      auto calcTextSize() -> wxSize;

      void onDatasetRowSelected(const DatasetEvent& event);
      void onSize(wxSizeEvent& event);
   };

}   // namespace ctb::app
