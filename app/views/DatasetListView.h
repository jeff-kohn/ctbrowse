#pragma once
#include "App.h"
#include "model/CtDataViewModel.h"

#include <ctb/model/ScopedEventSink.h>
#include <wx/menu.h>

namespace ctb::app
{

   class DatasetListView final : public wxDataViewCtrl, public IDatasetEventSink
   {
   public:
      /// @brief creates and initializes a panel for showing sort/filter options
      ///
      /// throws a ctb::Error if parent or source = nullptr, or if the window can't be created;
      /// otherwise returns a non-owning pointer to the window (parent window will manage 
      /// its lifetime). 
      /// 
      [[nodiscard]] static DatasetListView* create(wxWindow* parent, DatasetEventSourcePtr source);

   private:
      ScopedEventSink  m_sink;
      DataViewModelPtr m_model{};
      wxMenu           m_wine_menu{};
      wxMenu           m_pending_menu{};

      /// @brief private ctor used by static create()
      explicit DatasetListView(wxWindow* parent, DatasetEventSourcePtr source) : 
         wxDataViewCtrl{ parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME },
         m_sink { this, source },
         m_model{ CtDataViewModel::create() }
      {}

      void init();
      void buildWinePopup(wxMenu& menu, bool include_pending);
      void configureColumns();
      void setDataset(DatasetPtr dataset);
      void selectFirstRow();

      void notify(DatasetEvent event) override;
      void onSelectionChanged(wxDataViewEvent& event);
      void onWineContextMenu(wxDataViewEvent& event);

   };

} // namespace ctb::app