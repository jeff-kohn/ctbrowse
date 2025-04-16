#pragma once

#include "model/ScopedEventSink.h"

#include <wx/dataview.h>

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
      ScopedEventSink m_sink;
      DatasetPtr      m_dataset{};

      /// @brief private ctor used by static create()
      explicit DatasetListView(DatasetEventSourcePtr source) : m_sink{ this, source }
      {}

      // window creation
      void initControls();

      void setDataset(DatasetBase* dataset);

      // Inherited via IDatasetEventSink
      void notify(DatasetEvent event) override;
   };

} // namespace ctb::app