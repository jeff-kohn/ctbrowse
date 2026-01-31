/*******************************************************************
* @file SpinDoubleFilterCtrl.h
*
* @brief Header file for SpinDoubleFilterCtrl class
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/
#pragma once

#include "App.h"

#include <ctb/model/DatasetEventHandler.h>
#include <ctb/tables/CtSchema.h>

#include <wx/panel.h>
#include <wx/spinctrl.h>


class wxSpinCtrlDouble;
class wxCheckBox;

namespace ctb::app
{

   /// @brief UI component that combines a FilterCheckbox with a spin-control for a number filter value.
   ///
   class SpinDoubleFilterCtrl sealed : public wxPanel
   {
   public:
      using PropertyFilter = CtPropertyFilter;

      struct SpinParams
      {
         double   min_value{};
         double   max_value{};
         double   increment{};
         double   default_value{};
         uint16_t decimal_places{};
      };

      static auto create(wxWindow* parent, const DatasetEventSourcePtr& source, const PropertyFilter& filter, const SpinParams& params) -> SpinDoubleFilterCtrl*;

      /// @brief Get a reference to the filter associated with this control
      /// @return reference to the filter, which will have the appropriate cvref corresponding to 'this'
      template<typename Self>
      auto&& filter(this Self&& self)
      {
         return std::forward<Self>(self).m_filter;
      }

      auto enabled() const -> bool;

      void enable(bool enable);

      // no copy/move/assign, this class is created on the heap.
      SpinDoubleFilterCtrl() = delete;
      SpinDoubleFilterCtrl(const SpinDoubleFilterCtrl&) = delete;
      SpinDoubleFilterCtrl(SpinDoubleFilterCtrl&&) = delete;
      SpinDoubleFilterCtrl& operator=(const SpinDoubleFilterCtrl&) = delete;
      SpinDoubleFilterCtrl& operator=(SpinDoubleFilterCtrl&&) = delete;
      ~SpinDoubleFilterCtrl() override = default;

   private:
      DatasetEventHandler    m_event_handler;
      PropertyFilter         m_filter{};
      wxCheckBox*            m_checkbox{};
      wxSpinCtrlDouble*      m_spin{};

      SpinDoubleFilterCtrl(const DatasetEventSourcePtr& source, PropertyFilter filter) :
         m_event_handler(source),
         m_filter{ std::move(filter) }
      {}

      void createWindow(wxWindow* parent, const SpinParams& params);
      void onDatasetFilter(const DatasetEvent& event);
      void onDatasetInitialize(const DatasetEvent& event);
      void onFilterChecked(wxCommandEvent& event);
      void onSpinValueChanged(wxSpinDoubleEvent& event);
      void onSpinValueUpdateUI(wxUpdateUIEvent& event);
   };

} // namespace ctb::app