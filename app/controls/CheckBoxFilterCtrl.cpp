#include "CheckBoxFilterCtrl.h"

namespace ctb::app
{
   auto CheckBoxFilterCtrl::create(wxWindow* parent, const DatasetEventSourcePtr& source, const PropertyFilter& filter) -> CheckBoxFilterCtrl*
   {
      if (!parent)
      {
         assert("CheckBoxFilterCtrl parent pointer cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }

      std::unique_ptr<CheckBoxFilterCtrl> wnd{ new CheckBoxFilterCtrl{ source, filter } };
      wnd->createWindow(parent);
      return wnd.release(); // if we get here parent owns it, so return non-owning*
   }


   void CheckBoxFilterCtrl::createWindow(wxWindow* parent)
   {
      if (!Create(parent, wxID_ANY, wxFromSV(m_filter.filter_name)))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }

      SetValidator(wxGenericValidator{ &m_filter_enabled });

      Bind(wxEVT_CHECKBOX, &CheckBoxFilterCtrl::onFilterChecked, this);

      m_dataset_events.addHandler(DatasetEvent::Id::DatasetInitialize, [this](const DatasetEvent& event) { onDatasetFilter(event); });
      m_dataset_events.addHandler(DatasetEvent::Id::Filter,            [this](const DatasetEvent& event) { onDatasetFilter(event); });
   }


   void CheckBoxFilterCtrl::onFilterChecked(wxCommandEvent& event)
   {
      try
      {
         TransferDataFromWindow();

         auto&& dataset = m_dataset_events.getDataset(true);
         if (isEnabled())
         {
            dataset->propFilters().replaceFilter(m_filter.filter_name, m_filter);
         }
         else {
            dataset->propFilters().removeFilter(m_filter.filter_name);
         }
         m_dataset_events.signal_source(DatasetEvent::Id::Filter, false);
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
      
      auto filter = event.dataset->propFilters().getFilter(m_filter.filter_name);
      enable(filter.has_value() ? true : false);
   }

} // namespace ctb::app