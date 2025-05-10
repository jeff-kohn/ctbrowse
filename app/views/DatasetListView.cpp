#include "views/DatasetListView.h"
#include "wx_helpers.h"

#include <ctb/model/CtDataModel.h>
#include <ctb/model/DatasetLoader.h>

#include <wx/itemattr.h>
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
      assert(m_model.getDataset() and "This pointer should never be null here.");
      try
      {
         ClearColumns();
         auto& cols = m_model.getDataset()->displayColumns();
         for (const auto&& [idx, col] : vws::enumerate(cols))
         {
            AppendTextColumn(col.display_name.c_str(), 
               static_cast<uint32_t>(idx), wxDATAVIEW_CELL_INERT, 
               wxCOL_WIDTH_AUTOSIZE, static_cast<wxAlignment>(col.col_align));
         }
         wxPersistentRegisterAndRestore(this, wxFromSV(m_model.getDataset()->getTableName()));
      }
      catch (...) {
         wxGetApp().displayErrorMessage(packageError());
      }
   }


   void DatasetListView::setDataset(DatasetPtr dataset)
   {
      if (m_model.getDataset())
      {
         // save off current table's view settings. we'll restore/register again
         // (possibly for a different table) when we call configureColumns()
         wxPersistenceManager::Get().SaveAndUnregister(this);
      }
      wxWindowUpdateLocker freeze_updates{ this };

      // re-associate the model with the new dataset (nullptr is OK)
      m_model.setDataset(dataset);
      if (dataset)
      {
         // since we have a new dataset, re-initialize the list-view.
         configureColumns();
         m_model.Cleared();
         selectFirstRow();
      }
      else {
         m_model.Cleared();
      }
   }


   void DatasetListView::selectFirstRow()
   {
      throw Error{"Not implemented"};

      //if (0 == m_dataset->filteredRowCount()) return;

      //auto item = m_dataset->GetItem(0);
      //Select(item);
      //EnsureVisible(item);
      //SetFocus();
      //QueueEvent(new wxDataViewEvent{ wxEVT_DATAVIEW_SELECTION_CHANGED, this, nullptr, item });
   }


   void DatasetListView::notify(DatasetEvent event)
   {
      switch (event.event_id)
      {
         case DatasetEvent::Id::TableInitialize:
            m_model.setDataset(event.dataset);
            break;

         case DatasetEvent::Id::TableRemove:
            setDataset(DatasetPtr{});
            break;

         case DatasetEvent::Id::Sort:   [[fallthrough]];
         case DatasetEvent::Id::Filter: [[fallthrough]];
         case DatasetEvent::Id::SubStringFilter:
            m_model.Cleared(); // force a refresh, name is unfortunate
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

      /*auto row = m_sink.getTable()->GetRow(event.GetItem());
      m_sink.signal_source(DatasetEvent::Id::RowSelected, static_cast<int>(row));*/
   }


} // namespace ctb::app