/*********************************************************************
* @file       DatasetListView.cpp
*
* @brief      Implementation for the class DatasetListView
*
* @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
*********************************************************************/
#include "views/DatasetListView.h"

#include "MainFrame.h"
#include "wx_helpers.h"

#include <ctb/model/CtDataset.h>

#include <wx/persist/dataview.h>
#include <wx/wupdlock.h>

namespace ctb::app
{

   [[nodiscard]] DatasetListView* DatasetListView::create(wxWindow* parent, const DatasetEventSourcePtr& source)
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
      Bind(wxEVT_DATAVIEW_SELECTION_CHANGED,         &DatasetListView::onSelectionChanged, this);
      Bind(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, &DatasetListView::onWineContextMenu,  this);
      Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED,            &DatasetListView::onWineDoubleClick,  this);
   }


   // you should always call this when you need the ptr, don't cache/store the returned ptr because
   // it can become stale if new dataset colleciton is opened.
   auto getWinePopup() noexcept(false)-> MainFrame::wxMenuPtr
   {
      auto menu = wxGetApp().getMainWindow()->getWinePopupMenu();
      if (!menu)
         throw ctb::Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };

      return menu;
   }


   void DatasetListView::configureColumns()
   {
      assert(m_model->getDataset() and "This pointer should never be null here.");
      try
      {
         ClearColumns();
     
         auto cols = m_model->getDataset()->listColumns();
         for (const auto&& [idx, col] : vws::enumerate(cols))
         {
            AppendTextColumn(col.display_name.c_str(), 
               static_cast<uint32_t>(idx), wxDATAVIEW_CELL_INERT, 
               wxCOL_WIDTH_AUTOSIZE, static_cast<wxAlignment>(col.col_align));
         }
         wxPersistentRegisterAndRestore(this, wxFromSV(m_model->getDataset()->getTableName()));
      }
      catch (...) {
         wxGetApp().displayErrorMessage(packageError());
      }
   }


   void DatasetListView::setDataset(const DatasetPtr& dataset)
   {
      if (m_model->getDataset())
      {
         // save off current table's view settings. we'll restore/register again
         // (possibly for a different table) when we call configureColumns()
         wxPersistenceManager::Get().SaveAndUnregister(this);
      }
      wxWindowUpdateLocker freeze_updates{ this };

      // re-associate the model with the new dataset (nullptr is OK)
      m_model->setDataset(dataset);
      m_model->associateView(this);
      if (dataset)
      {
         // since we have a new dataset, re-initialize the list-view.
         configureColumns();
         m_model->reQuery();
         selectFirstRow();
      }
      else {
         m_model->reQuery();
      }
   }


   void DatasetListView::selectFirstRow()
   {
      auto dataset = m_model->getDataset();
      if (!dataset or dataset->rowCount() == 0)
         return;

      auto item = m_model->GetItem(0);
      Select(item);
      EnsureVisible(item);
      SetFocus();
      QueueEvent(new wxDataViewEvent{ wxEVT_DATAVIEW_SELECTION_CHANGED, this, nullptr, item });
   }


   void DatasetListView::notify(DatasetEvent event)
   {
      switch (event.event_id)
      {
         case DatasetEvent::Id::DatasetInitialize:
            setDataset(event.dataset);
            break;

         case DatasetEvent::Id::DatasetRemove:
            setDataset(DatasetPtr{});
            break;

         case DatasetEvent::Id::Sort:   [[fallthrough]];
         case DatasetEvent::Id::Filter: [[fallthrough]];
         case DatasetEvent::Id::SubStringFilter:
            m_model->reQuery();
            selectFirstRow();
            break;

         case DatasetEvent::Id::RowSelected:
            break;

         default:
            assert("Unexpected event type" and false);
      }
   }


   void DatasetListView::onSelectionChanged(wxDataViewEvent& event)
   {
      try 
      {
         if (!m_sink.hasDataset()) return;

         auto row = static_cast<int>(m_model->GetRow(event.GetItem()));
         if (row >= 0)
            m_sink.signal_source(DatasetEvent::Id::RowSelected, false, row);
      }
      catch (...) {
         wxGetApp().displayErrorMessage(packageError());
      }
   }


   void DatasetListView::onWineContextMenu(wxDataViewEvent& event)
   {
      try
      {
         if (!m_sink.hasDataset())
            event.Skip();

         auto popup = getWinePopup();
         PopupMenu(popup.get());
      }
      catch (...) {
         wxGetApp().displayErrorMessage(packageError());
      }
   }


   void DatasetListView::onWineDoubleClick(wxDataViewEvent& event)
   {
      try
      {
         if (event.GetItem().IsOk() and m_sink.hasDataset())
         {
			   QueueEvent(new wxCommandEvent{ wxEVT_COMMAND_MENU_SELECTED, CMD_ONLINE_WINE_DETAILS });
         }
         else {
            event.Skip();
         }
      }
      catch (...) {
         wxGetApp().displayErrorMessage(packageError());
      }

   }


} // namespace ctb::app