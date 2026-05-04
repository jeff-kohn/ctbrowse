#include "CheckBoxFilterCtrl.h"

namespace ctb::app
{
   auto CheckBoxFilterCtrl::create(wxWindow* parent, const DatasetEventSourcePtr& source, const PropertyFilter& filter) -> CheckBoxFilterCtrl*
   {
      return detail::createDatasetWindow<CheckBoxFilterCtrl>(parent, source, filter);
   }


   void CheckBoxFilterCtrl::createWindow(wxWindow* parent)
   {
      if (!Create(parent, wxID_ANY, wxFromSV(m_filter.filter_name)))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }

      SetValidator(wxGenericValidator{ &m_filter_enabled });

      Bind(wxEVT_CHECKBOX, &CheckBoxFilterCtrl::onFilterChecked, this);

      getEventHandler().addHandler(DatasetEvent::Id::DatasetInitialize, [this](const DatasetEvent& event) { onDatasetFilter(event); });
      getEventHandler().addHandler(DatasetEvent::Id::Filter,            [this](const DatasetEvent& event) { onDatasetFilter(event); });
   }


   void CheckBoxFilterCtrl::onFilterChecked([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         TransferDataFromWindow();

         auto&& dataset = getDataset();
         if (isEnabled())
         {
            dataset->propFilters().replaceFilter(m_filter.filter_name, m_filter);
         }
         else {
            dataset->propFilters().removeFilter(m_filter.filter_name);
         }
         getEventHandler().signal_source(DatasetEvent::Id::Filter, false);
      }
      catch (...) {
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void CheckBoxFilterCtrl::enable(bool enable)
   {
      m_filter_enabled = enable;
      TransferDataToWindow();
   }


   void CheckBoxFilterCtrl::onDatasetFilter(const DatasetEvent& event)
   {
      assert(event.dataset);
      
      // cppcheck-suppress [shadowFunction] 
      auto filter = event.dataset->propFilters().getFilter(m_filter.filter_name);
      enable(filter.has_value() ? true : false);
   }

} // namespace ctb::app