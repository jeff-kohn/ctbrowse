#pragma once
#include "App.h"

#include <ctb/model/DatasetEventHandler.h>
#include <ctb/tables/CtSchema.h>
#include <wx/checkbox.h>
#include <wx/valgen.h>

namespace ctb::app
{

   /// @brief Really simple control class that binds a CtPropertyFilter to a checkbox control
   class CheckBoxFilterCtrl final : public wxCheckBox
   {
   public:
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

      // no copy/move/assign, this class is created on the heap and passed around as ptr
      CheckBoxFilterCtrl(const CheckBoxFilterCtrl&) = delete;
      CheckBoxFilterCtrl(CheckBoxFilterCtrl&&) = delete;
      CheckBoxFilterCtrl& operator=(const CheckBoxFilterCtrl&) = delete;
      CheckBoxFilterCtrl& operator=(CheckBoxFilterCtrl&&) = delete;
      ~CheckBoxFilterCtrl() noexcept override = default;

   private:
      DatasetEventHandler m_dataset_events;
      PropertyFilter      m_filter{};
      bool                m_filter_enabled{false};

      CheckBoxFilterCtrl(const DatasetEventSourcePtr& source, PropertyFilter filter) : 
         m_dataset_events{source },
         m_filter{ std::move(filter) }
      {}

      void createWindow(wxWindow* parent);
      void onFilterChecked(wxCommandEvent& event);
      void onDatasetFilter(const DatasetEvent& event);
   };

};