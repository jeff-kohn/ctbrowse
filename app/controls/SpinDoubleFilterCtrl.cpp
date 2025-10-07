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
   auto SpinDoubleFilterCtrl::create(wxWindow& parent, const DatasetEventSourcePtr& source, 
                                     const PropertyFilter& filter, const SpinParams& params) -> SpinDoubleFilterCtrl*
   {
      // non-owning pointer, parent window manages lifetime.
      return new SpinDoubleFilterCtrl{ parent, source, filter, params };
   }

    
   SpinDoubleFilterCtrl::SpinDoubleFilterCtrl(wxWindow& parent, DatasetEventSourcePtr source, PropertyFilter filter, const SpinParams& params) : 
      wxPanel{ &parent },
      m_event_handler{ std::move(source) },
      m_filter{ std::move(filter) }
   {
      m_filter.enabled = false;
      initControls(params);
      m_event_handler.addHandler(DatasetEvent::Id::DatasetInitialize, [this](const DatasetEvent& event) { onDatasetInitialize(event); });
      m_event_handler.addHandler(DatasetEvent::Id::Filter,            [this](const DatasetEvent& event) { onDatasetFilter(event);     });
   }


   auto SpinDoubleFilterCtrl::enabled() const -> bool
   {
      return m_filter.enabled;
   }


   void SpinDoubleFilterCtrl::enable(bool enable)
   {
     m_filter.enabled = enable;
   }


   void SpinDoubleFilterCtrl::initControls(const SpinParams& params)
   {
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
         params.min_value, params.max_value, params.default_value, params.increment 
      };
      m_spin->SetDigits(params.decimal_places);
      sizer->Add(m_spin, wxSizerFlags{});


      m_spin->Bind(wxEVT_SPINCTRLDOUBLE, &SpinDoubleFilterCtrl::onSpinValueChanged,  this);
      m_spin->Bind(wxEVT_UPDATE_UI,      &SpinDoubleFilterCtrl::onSpinValueUpdateUI, this);    
      m_checkbox->Bind(wxEVT_CHECKBOX,   &SpinDoubleFilterCtrl::onFilterChecked,     this);
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


   void SpinDoubleFilterCtrl::onDatasetInitialize(const DatasetEvent& event)
   {
      auto&& filter = event.dataset->propFilters().getFilter(m_filter.filter_name);
      if (filter)
      {
         m_filter = *filter;
      }
      TransferDataToWindow();
   }


   void SpinDoubleFilterCtrl::onFilterChecked(wxCommandEvent& event)
   {
      try
      {
         TransferDataFromWindow();

         auto&& dataset = m_event_handler.getDataset();
         if (m_filter.enabled)
         {
            dataset->propFilters().replaceFilter(m_filter.filter_name, m_filter);
         }
         else {
            dataset->propFilters().removeFilter(m_filter.filter_name);
         }
         m_event_handler.signal_source(DatasetEvent::Id::Filter, false);
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void SpinDoubleFilterCtrl::onSpinValueChanged(wxSpinDoubleEvent& event)
   {
      try
      {
         TransferDataFromWindow();

         auto dataset = m_event_handler.getDataset();
         m_filter.compare_val = event.GetValue();
         if (m_filter.enabled)
         {
            dataset->propFilters().replaceFilter(m_filter.filter_name, m_filter);
            m_event_handler.signal_source(DatasetEvent::Id::Filter, false);
         }
      }
      catch(...){
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }

   void SpinDoubleFilterCtrl::onSpinValueUpdateUI(wxUpdateUIEvent& event)
   {
      event.Enable(m_filter.enabled);
   }

} // namespace ctb::app