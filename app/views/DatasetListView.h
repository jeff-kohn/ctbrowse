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

#include <ctb/model/ScopedEventSink.h>
#include <wx/menu.h>

#include <memory>

namespace ctb::app
{
   /// @brief Panel view class for displaying all the wines in a collection in list-view format.
   ///
   class DatasetListView final : public wxDataViewCtrl, public IDatasetEventSink
   {
   public:
      /// @brief creates and initializes a panel for showing sort/filter options
      ///
      /// throws a ctb::Error if parent or source = nullptr, or if the window can't be created;
      /// otherwise returns a non-owning pointer to the window (parent window will manage 
      /// its lifetime). 
      [[nodiscard]] static DatasetListView* create(wxWindow* parent, DatasetEventSourcePtr source);

   private:
      using wxMenuPtr = std::unique_ptr<wxMenu>;

      DataViewModelPtr m_model{};
      ScopedEventSink  m_sink;
      wxMenuPtr        m_popup_menu{};

      /// @brief private ctor used by static create()
      explicit DatasetListView(wxWindow* parent, DatasetEventSourcePtr source) : 
         wxDataViewCtrl{ parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME },
         m_model{ CtDataViewModel::create() },
         m_sink { this, source }
      {}

      void init();
      void buildWinePopup(TableId table_id);
      auto getWinePopup() noexcept(false) -> wxMenu*; // will throw if we don't have a non-nullptr wxMenu to return
      void configureColumns();
      void setDataset(DatasetPtr dataset);
      void selectFirstRow();

      void notify(DatasetEvent event) override;
      void onSelectionChanged(wxDataViewEvent& event);
      void onWineContextMenu(wxDataViewEvent& event);

   };

} // namespace ctb::app