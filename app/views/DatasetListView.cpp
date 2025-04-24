
#include "model/CtDataModel.h"
#include "views/DatasetListView.h"
#include "model/DatasetLoader.h"

#include <wx/persist/dataview.h>
#include <wx/wupdlock.h>

namespace ctb::app
{

   [[nodiscard]] DatasetListView* DatasetListView::create(wxWindow* parent, DatasetEventSourcePtr source)
   {
      if (!source)
      {
         assert("source parameter cannot == nullptr" and false);
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }
      if (!parent)
      {
         assert("parent parameter cannot == nullptr" and false);
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }

      std::unique_ptr<DatasetListView> wnd{ new DatasetListView{ parent, source } };
      wnd->init();
      return wnd.release(); // parent owns child, so we don't need to delete
   }


   void DatasetListView::init()
   {
      Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &DatasetListView::onSelectionChanged, this);
   }


   void DatasetListView::configureColumns()
   {
      assert(m_dataset.get() and "This pointer should never be null here.");
      try
      {
         ClearColumns();
         auto cols = m_dataset->defaultDisplayColumns();
         for (const auto&& [idx, col] : vws::enumerate(cols))
         {
            AppendTextColumn(col.display_name.c_str(), 
               static_cast<uint32_t>(idx), wxDATAVIEW_CELL_INERT, 
               wxCOL_WIDTH_AUTOSIZE, static_cast<wxAlignment>(col.col_align));
         }
         wxPersistentRegisterAndRestore(this, wxFromSV(m_dataset->getTableName()));
      }
      catch (...) {
         wxGetApp().displayErrorMessage(packageError());
      }
   }


   void DatasetListView::setDataset(DatasetPtr dataset)
   {
      if (m_dataset)
      {
         // save off current table's view settings. we'll restore/register again
         // (possibly for a different table) when we call configureColumns()
         wxPersistenceManager::Get().SaveAndUnregister(this);
      }
      wxWindowUpdateLocker freeze_updates{ this };

      // re-associate this class with the new model* (nullptr is OK)
      m_dataset = dataset;
      AssociateModel(m_dataset.get());
      if (!m_dataset)
         return;

      // since we have a new model*, re-initialize the listview.
      configureColumns();
      m_dataset->Cleared();
      selectFirstRow();
   }


   void DatasetListView::selectFirstRow()
   {
      if (0 == m_dataset->filteredRowCount()) return;

      auto item = m_dataset->GetItem(0);
      Select(item);
      EnsureVisible(item);
      SetFocus();
      QueueEvent(new wxDataViewEvent{ wxEVT_DATAVIEW_SELECTION_CHANGED, this, nullptr, item });
   }


   void DatasetListView::notify(DatasetEvent event)
   {
      switch (event.m_event_id)
      {
         case DatasetEvent::Id::TableInitialize:
            setDataset(event.m_data);
            break;

         case DatasetEvent::Id::TableRemove:
            setDataset(DatasetPtr{});
            break;

         case DatasetEvent::Id::Sort:   [[fallthrough]];
         case DatasetEvent::Id::Filter: [[fallthrough]];
         case DatasetEvent::Id::SubStringFilter:
            m_dataset->Cleared();
            selectFirstRow();
            break;

         case DatasetEvent::Id::ColLayoutRequested: [[fallthrough]];
         case DatasetEvent::Id::RowSelected:
            break;

         default:
            assert("Unexpected event type" and false);
      }
   }


   void DatasetListView::onSelectionChanged(wxDataViewEvent& event)
   {
      if (!m_sink.hasTable()) return;

      auto row = m_sink.getTable()->GetRow(event.GetItem());
      m_sink.signal_source(DatasetEvent::Id::RowSelected, static_cast<int>(row));
   }


} // namespace ctb::app