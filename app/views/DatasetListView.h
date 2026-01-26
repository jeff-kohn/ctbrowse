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
      /// @brief creates and initializes a panel for showing sort/filter options
      ///
      /// throws a ctb::Error if parent or source = nullptr, or if the window can't be created;
      /// otherwise returns a non-owning pointer to the window (parent window will manage 
      /// its lifetime). 
      [[nodiscard]] static DatasetListView* create(wxWindow* parent, const DatasetEventSourcePtr& source);

   private:
      DataViewModelPtr m_model{};
      DatasetEventHandler  m_event_handler;

      /// @brief private ctor used by static create()
      explicit DatasetListView(wxWindow* parent, DatasetEventSourcePtr source) : 
         wxDataViewCtrl{ parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME },
         m_model{ CtDataViewModel::create() },
         m_event_handler { std::move(source) }
      {}

      void init();
      void configureColumns();
      void setDataset(const DatasetPtr& dataset);
      void selectFirstRow();

      void onDatasetEvent(DatasetEvent event);
      void onSelectionChanged(wxDataViewEvent& event);
      void onWineContextMenu(wxDataViewEvent& event);
      void onWineDoubleClick(wxDataViewEvent& event);

   };

} // namespace ctb::app