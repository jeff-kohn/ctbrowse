#pragma once

#include "App.h"

#include <ctb/model/DatasetEventHandler.h>
#include <wx/panel.h>


class wxChoice;

namespace ctb::app
{
   class SortOptionsPanel final : public wxPanel
   {
   public:
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> SortOptionsPanel*;

   private:
      DatasetEventHandler   m_dataset_events;            // for handling dataset events
      IDataset::TableSort   m_sort_config{};             // the sort object that will be used to sort the dataset 
      int                   m_sort_selection{ 0 };       // index of selected sort in combo, which matches a sort in availableSorts()
      bool                  m_sort_ascending{ true };    // whether ascending sort order is active
      bool                  m_sort_descending{ false };  // whether descending sort ordes is active (yes we need both)
      wxChoice*             m_sort_combo{};

      SortOptionsPanel(const DatasetEventSourcePtr& source) : m_dataset_events(source)
      {}

      void createWindow(wxWindow* parent);
      void onSortOrderClicked(wxCommandEvent& event);
      void onSortSelection(wxCommandEvent& event);
      void onTableSorted(DatasetEvent event);
      void onDatasetInitialize(DatasetEvent event);
   };


}