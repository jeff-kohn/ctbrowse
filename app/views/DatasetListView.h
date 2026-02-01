/*********************************************************************
* @file       DatasetListView.h
*
* @brief      Declaration for the class DatasetListView
*
* @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
*********************************************************************/
#pragma once

#include "App.h"
#include "model/CtDataViewModel.h"

#include <ctb/model/DatasetEventHandler.h>

namespace ctb::app
{
   /// @brief Panel view class for displaying all the wines in a collection in list-view format.
   ///
   class DatasetListView final : public wxDataViewCtrl
   {
   public:
      /// @brief creates and initializes a panel containing a list view of dataset rows
      ///
      /// throws a ctb::Error if parent or source = nullptr, or if the window can't be created;
      /// otherwise returns a non-owning pointer to the window (parent window will manage 
      /// its lifetime). 
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) -> DatasetListView*;

      DatasetListView() = delete;
      DatasetListView(const DatasetListView&) = delete;
      DatasetListView(DatasetListView&&) = delete;
      DatasetListView& operator=(const DatasetListView&) = delete;
      DatasetListView& operator=(DatasetListView&&) = delete;
      ~DatasetListView() noexcept override = default;

   private:
      DatasetEventHandler  m_dataset_events;
      DataViewModelPtr     m_model{};

      /// @brief private ctor used by static create()
      explicit DatasetListView(DatasetEventSourcePtr source) : 
         m_dataset_events { std::move(source) },
         m_model{ CtDataViewModel::create() }
      {}

      void createWindow(wxWindow* parent);
      void configureColumns();
      void setDataset(const DatasetPtr& dataset);
      void selectFirstRow();

      void onDatasetEvent(DatasetEvent event);
      void onSelectionChanged(wxDataViewEvent& event);
      void onWineContextMenu(wxDataViewEvent& event);
      void onWineDoubleClick(wxDataViewEvent& event);

   };

} // namespace ctb::app