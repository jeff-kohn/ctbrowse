#include "SortOptionsPanel.h"

#include <wx/choice.h>
#include <wx/radiobut.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/valgen.h>

namespace ctb::app
{

   namespace
   {
      auto getSortOptionList(DatasetPtr dataset) -> wxArrayString
      {
         return vws::all(dataset->availableSorts())
            | vws::transform([](const IDataset::TableSort& s) {  return wxFromSV(s.sort_name); })
            | rng::to<wxArrayString>();
      }
   }

   auto SortOptionsPanel::create(wxWindow* parent, const DatasetEventSourcePtr& source) -> SortOptionsPanel*
   {
      if (!parent)
      {
         assert("parent pointer cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }
      if (!source)
      {
         assert("source parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }

      std::unique_ptr<SortOptionsPanel> wnd{ new SortOptionsPanel{source} };
      wnd->createWindow(parent);
      return wnd.release(); // parent owns child, so we don't need to delete
   }


   void SortOptionsPanel::createWindow(wxWindow* parent)
   {
      using namespace ctb::constants;
      
      if (!Create(parent))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }

      // sort options box
      auto* sort_options_box = new wxStaticBoxSizer(wxVERTICAL, this, LBL_SORT_OPTIONS);
      SetSizer(sort_options_box);

      // sort fields combo
      m_sort_combo = new wxChoice(sort_options_box->GetStaticBox(), wxID_ANY);
      m_sort_combo->SetFocus();
      m_sort_combo->SetValidator(wxGenericValidator(&m_sort_selection));
      sort_options_box->Add(m_sort_combo, wxSizerFlags{}.Expand().Border(wxALL));

      // ascending sort order radio. 
      auto opt_ascending = new wxRadioButton{
         sort_options_box->GetStaticBox(),
         wxID_ANY,
         LBL_SORT_ASCENDING,
         wxDefaultPosition,
         wxDefaultSize,
         wxRB_GROUP
      };
      opt_ascending->SetValue(true);
      opt_ascending->SetValidator(wxGenericValidator{ &m_sort_ascending });
      sort_options_box->Add(opt_ascending, wxSizerFlags{}.Expand().Border(wxALL));

      // descending sort order radio. Since the radio buttons aren't in a group box, the validator treats them as individual bools
      // so we have a separate flag for the descending radio that we have to manually keep in sync (see onTableSorted)
      auto opt_descending = new wxRadioButton{ sort_options_box->GetStaticBox(), wxID_ANY, LBL_SORT_DESCENDING };
      opt_descending->SetValidator(wxGenericValidator{ &m_sort_descending });
      sort_options_box->Add(opt_descending, wxSizerFlags{ 1 }.Expand().Border(wxALL));

      // event bindings.
      m_sort_combo->Bind(wxEVT_CHOICE, &SortOptionsPanel::onSortSelection, this);
      opt_ascending->Bind(wxEVT_RADIOBUTTON, &SortOptionsPanel::onSortOrderClicked, this);
      opt_descending->Bind(wxEVT_RADIOBUTTON, &SortOptionsPanel::onSortOrderClicked, this);

      m_dataset_events.addHandler(DatasetEvent::Id::DatasetInitialize, [this](const DatasetEvent& event) { onDatasetInitialize(event);  });
      m_dataset_events.addHandler(DatasetEvent::Id::Sort,              [this](const DatasetEvent& event) { onTableSorted(event);        });
   }


   void SortOptionsPanel::onSortOrderClicked([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         TransferDataFromWindow();

         auto dataset = m_dataset_events.getDataset(true);
         m_sort_config.reverse = m_sort_descending;
         dataset->applySort(m_sort_config);
         m_dataset_events.signal_source(DatasetEvent::Id::Sort, false);
      }
      catch (...) {
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void SortOptionsPanel::onSortSelection([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         // event could get generated even if they didn't change the selection, don't waste our time.
         auto old_index = m_sort_selection;
         TransferDataFromWindow();
         if (old_index == m_sort_selection)
            return;

         // let the combo close its list before we reload the dataset
         CallAfter([this]()
            {
               auto dataset = m_dataset_events.getDataset(true);
               auto sorts = dataset->availableSorts();
               if (m_sort_selection <= std::ssize(sorts))
               {
                  // re-fetch sorter based on index. UI and member state will get updated in the dataset event handler.
                  dataset->applySort(sorts[static_cast<size_t>(m_sort_selection)]);
                  m_dataset_events.signal_source(DatasetEvent::Id::Sort, true);
               }
               else {
                  log::warn("SortOptionsPanel::onSortSelection: invalid sort index selected: {}", m_sort_selection);
               }
            });
      }
      catch (...) {
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }


   void SortOptionsPanel::onDatasetInitialize(DatasetEvent event)
   {
      assert(event.dataset);
    
      m_sort_combo->Clear();
      m_sort_combo->Append(getSortOptionList(event.dataset));
      onTableSorted(event); // a bit hacky but techincally correct.
   }

   void SortOptionsPanel::onTableSorted(DatasetEvent event)
   {
      assert(event.dataset);
      try
      {
         auto& dataset = event.dataset;
         m_sort_config = dataset->activeSort();
         m_sort_ascending = (m_sort_config.reverse == false);
         m_sort_descending = m_sort_config.reverse;

         for (const auto&& [idx, sort] : vws::enumerate(dataset->availableSorts()))
         {
            if (m_sort_config.sort_name == sort.sort_name)
            {
               m_sort_selection = idx;
            }
         }
         TransferDataToWindow();
      }
      catch (...) {
         wxGetApp().displayErrorMessage(packageError(), true);
      }
   }

}