#pragma once
#include "App.h"
#include "model/DatasetWindow.h"

#include <ctb/model/DatasetEventHandler.h>
#include <ctb/tables/CtSchema.h>
#include <wx/checkbox.h>
#include <wx/valgen.h>

namespace ctb::app
{

   /// @brief Really simple control class that binds a CtPropertyFilter to a checkbox control
   class CheckBoxFilterCtrl final : public DatasetWindow<wxCheckBox>
   {
   public:
      using Base           = DatasetWindow<wxCheckBox>;
      using PropertyFilter = CtPropertyFilter;

      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source, const PropertyFilter& filter) -> CheckBoxFilterCtrl*;

      /// @brief Get a reference to the filter associated with this control
      /// @return reference to the filter, which will have the appropriate cvref corresponding to 'this'
      template<typename Self>
      auto&& filter(this Self&& self)     {  return std::forward<Self>(self).m_filter;    }

      /// @brief Enables or disables the filter
      void enable(bool enable = true);

      /// @return whether or not the filter is currently applied to the dataset
      auto isEnabled() const -> bool      {  return m_filter_enabled;                     }

   private:
      PropertyFilter      m_filter{};
      bool                m_filter_enabled{false};

      CheckBoxFilterCtrl(const DatasetEventSourcePtr& source, PropertyFilter filter) : 
         Base{ source },
         m_filter{ std::move(filter) }
      {}

      // this class can only be constructed through static create(), which uses createDetailsViewFactory to call protected ctor
      template<typename WndT, typename... Args>
      friend auto detail::createDatasetWindow(wxWindow* parent, const DatasetEventSourcePtr& source, Args&&... args)->WndT*;

      void createWindow(wxWindow* parent);
      void onFilterChecked(wxCommandEvent& event);
      void onDatasetFilter(const DatasetEvent& event);
   };

};