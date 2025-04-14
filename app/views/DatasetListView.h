#pragma once

#include "model/ScopedEventSink.h"

#include <wx/dataview.h>

namespace ctb::app
{

   class DatasetListView final : public wxDataViewCtrl, public IDatasetEventSink
   {
   public:
      /// @brief creates and initializes a panel for showing grid sort/filter options
      ///
      /// throws a ctb::Error if parent or source = nullptr, or if the window can't be created;
      /// otherwise returns a non-owning pointer to the window (parent window will manage 
      /// its lifetime). 
      /// 
      [[nodiscard]] static DatasetListView* create(wxWindow* parent, DatasetEventSourcePtr source);

   private:
      ScopedEventSink m_sink;

      // window creation
      void initControls();

      /// @brief private ctor used by static create()
      explicit DatasetListView(DatasetEventSourcePtr source);

      // Inherited via IDatasetEventSink
      void notify(DatasetEvent event) override;
   };

} // namespace ctb::app