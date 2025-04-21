
#include "model/CtDataModel.h"
#include "views/DatasetListView.h"
#include "model/DatasetLoader.h"

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

      std::unique_ptr<DatasetListView> wnd{ new DatasetListView{source} };
      if (!wnd->Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }
      return wnd.release(); // parent owns child, so we don't need to delete
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
            AppendTextColumn(col.display_name.c_str(), static_cast<uint32_t>(idx));
         }
      }
      catch (...) {
         wxGetApp().displayErrorMessage(packageError());
      }
   }


   void DatasetListView::setDataset(DatasetPtr dataset)
   {
      m_dataset = dataset;
      AssociateModel(m_dataset.get());
      if (m_dataset)
      {
         configureColumns();
         m_dataset->Reset(m_dataset->GetCount());
         m_dataset->IncRef();// wxWidgets is lame
         dataset->Cleared();
      }
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
            m_dataset->Reset(m_dataset->GetCount());
            break;

         case DatasetEvent::Id::ColLayoutRequested: [[fallthrough]];
         case DatasetEvent::Id::RowSelected:
            break;

         default:
            assert("Unexpected event type" and false);
      }
   }


} // namespace ctb::app