/*******************************************************************
 * @file DatasetOptionsView.h
 *
 * @brief Header file for DatasetOptionsView class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "App.h"
#include "model/CtDatasetOptions.h"

#include <ctb/model/DatasetEventHandler.h>

#include <wx/panel.h>
#include <wx/spinctrl.h>


// forward declare member pointers to avoid header pollution.
class wxBoxSizer;
class wxChoice;
class wxStaticBoxSizer;
class wxStaticText;
class wxSlider;


namespace ctb::app
{

   class CheckBoxFilterCtrl;
   class SpinDoubleFilterCtrl;
   class MultiValueFilterTreeCtrl;


   /// @brief panel class that provides UI for setting sorting and filtering options
   ///
   class DatasetOptionsView final : public wxPanel
   {
   public:
      /// @brief creates and initializes a panel for showing sort/filter options
      ///
      /// throws a ctb::Error if source == nullptr, or if the window can't be created;
      /// otherwise returns a non-owning pointer to the window
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source) noexcept(false) -> DatasetOptionsView* ;


      // no copy/move/assign, this class is created on the heap.
      DatasetOptionsView(const DatasetOptionsView&) = delete;
      DatasetOptionsView(DatasetOptionsView&&) = delete;
      DatasetOptionsView& operator=(const DatasetOptionsView&) = delete;
      DatasetOptionsView& operator=(DatasetOptionsView&&) = delete;
      ~DatasetOptionsView() override = default;
      
   private:
      bool                      m_sort_ascending{ true  };     // whether ascending sort order is active
      bool                      m_sort_descending{ false };    // whether descending sort ordes is active (yes we need both)
      int                       m_sort_selection{ 0 };         // index of selected sort in combo, which matches a sort in availableSorts()
      DatasetEventHandler       m_dataset_events; 
      MultiValueFilterTreeCtrl* m_filter_tree{};
      StringSet                 m_supported_filters{};        // set of filter names that we have controls for
      wxStaticText*             m_dataset_title{};

      // window creation
      void createWindow(wxWindow* parent);
      void createOptionFilters(wxStaticBoxSizer* parent);

      // Dataset-related event handlers
      void onDatasetInitialize(DatasetEvent event);

      /// @brief private ctor used by static create()
      explicit DatasetOptionsView(const DatasetEventSourcePtr& source);
   };

} // namespace ctb::app