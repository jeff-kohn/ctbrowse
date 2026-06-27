#pragma once

#include "App.h"
#include "model/DatasetWindow.h"

#include <ctb/model/DatasetEventHandler.h>
#include <wx/panel.h>


class wxChoice;

namespace ctb::app
{
   class SortOptionsPanel final : public DatasetWindow<wxPanel>
   {
   public:
      using Base = DatasetWindow<wxPanel>;

      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> SortOptionsPanel*;

   private:
      IDataset::TableSort m_sort_config{};              // the sort object that will be used to sort the dataset
      int                 m_sort_selection{ 0 };        // index of selected sort in combo, which matches a sort in availableSorts()
      bool                m_sort_ascending{ true };     // whether ascending sort order is active
      bool                m_sort_descending{ false };   // whether descending sort ordes is active (yes we need both)
      wxChoice*           m_sort_combo{};

      DECLARE_DATASET_WINDOW_FACTORY;

      SortOptionsPanel(const DatasetEventSourcePtr& source) : Base{ source }
      {}

      void createWindow(wxWindow* parent) override;
      void onSortOrderClicked(wxCommandEvent& event);
      void onSortSelection(wxCommandEvent& event);
      void onTableSorted(const DatasetEvent& event);
      void onDatasetInitialize(const DatasetEvent& event);
   };


}   // namespace ctb::app
