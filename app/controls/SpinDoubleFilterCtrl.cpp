/*******************************************************************
* @file SpinDoubleFilterCtrl.cpp
*
* @brief Implementation file for SpinDoubleFilterCtrl class
* 
* @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
*******************************************************************/

#include "SpinDoubleFilterCtrl.h"

#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/valgen.h>

namespace ctb::app
{
   [[nodiscard]] auto SpinDoubleFilterCtrl::create(wxWindow* parent, const DatasetEventSourcePtr& source,  const PropertyFilter& filter, const SpinParams& params) -> SpinDoubleFilterCtrl*
   {
      return detail::createDatasetWindow<SpinDoubleFilterCtrl>(parent, source, filter, params);
   }


   auto SpinDoubleFilterCtrl::enabled() const -> bool
   {
      return m_filter.enabled;
   }


   void SpinDoubleFilterCtrl::enable(bool enable)
   {
     m_filter.enabled = enable;
   }


   void SpinDoubleFilterCtrl::createWindow(wxWindow* parent) 
   {
      if (!Create(parent))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }

      m_filter.enabled = false;
      SetSizer(new wxBoxSizer{ wxHORIZONTAL });
      auto* sizer = GetSizer();

      m_checkbox = new wxCheckBox{ this, wxID_ANY, m_filter.filter_name };
      m_checkbox->SetValidator(wxGenericValidator{ &m_filter.enabled });
      sizer->Add(m_checkbox, wxSizerFlags{1}.Expand());

      m_spin = new wxSpinCtrlDouble
      { 
         this, wxID_ANY, 
         wxEmptyString, wxDefaultPosition, wxDefaultSize, 
         wxSP_ARROW_KEYS | wxALIGN_RIGHT,
         m_spin_params.min_value, m_spin_params.max_value, m_spin_params.default_value, m_spin_params.increment
      };
      m_spin->SetDigits(m_spin_params.decimal_places);
      sizer->Add(m_spin, wxSizerFlags{});


      m_spin->Bind(wxEVT_SPINCTRLDOUBLE, &SpinDoubleFilterCtrl::onSpinValueChanged,  this);
      m_spin->Bind(wxEVT_UPDATE_UI,      &SpinDoubleFilterCtrl::onSpinValueUpdateUI, this);    
      m_checkbox->Bind(wxEVT_CHECKBOX,   &SpinDoubleFilterCtrl::onFilterChecked,     this);

      getEventHandler().addHandler(DatasetEvent::Id::DatasetInitialize, [this](const DatasetEvent& event) { onDatasetInitialize(event); });
      getEventHandler().addHandler(DatasetEvent::Id::DatasetFiltered, [this](const DatasetEvent& event) { onDatasetFilter(event);     });
   }


   void SpinDoubleFilterCtrl::onDatasetFilter(const DatasetEvent& event)
   {
      auto&& filter = event.dataset->propFilters().getFilter(m_filter.filter_name);
      if (filter)
      {
         m_filter = *filter;
      }
      else {
         // it's possible this filter was cleared/removed from toolbar, which is why it wasn't found.
         m_filter.enabled = false;
      }
      TransferDataToWindow();
   }


   void SpinDoubleFilterCtrl::onDatasetInitialize([[maybe_unused]] const DatasetEvent& event)
   {
      auto&& filter = event.dataset->propFilters().getFilter(m_filter.filter_name);
      if (filter)
      {
         m_filter = *filter;
      }
      TransferDataToWindow();
   }


   void SpinDoubleFilterCtrl::onFilterChecked([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         TransferDataFromWindow();

         auto&& dataset = getEventHandler().getDataset();
         if (m_filter.enabled)
         {
            dataset->propFilters().replaceFilter(m_filter.filter_name, m_filter);
         }
         else {
            dataset->propFilters().removeFilter(m_filter.filter_name);
         }
         getEventHandler().signal_source(DatasetEvent::Id::DatasetFiltered, false);
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void SpinDoubleFilterCtrl::onSpinValueChanged([[maybe_unused]] wxSpinDoubleEvent& event)
   {
      try
      {
         TransferDataFromWindow();

         auto dataset = getEventHandler().getDataset();
         m_filter.compare_val = event.GetValue();
         if (m_filter.enabled)
         {
            dataset->propFilters().replaceFilter(m_filter.filter_name, m_filter);
            getEventHandler().signal_source(DatasetEvent::Id::DatasetFiltered, false);
         }
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }

   void SpinDoubleFilterCtrl::onSpinValueUpdateUI([[maybe_unused]] wxUpdateUIEvent& event)
   {
      event.Enable(m_filter.enabled);
   }

} // namespace ctb::app
