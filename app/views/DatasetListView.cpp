
#include "model/CtDataModel.h"
#include "views/DatasetListView.h"
#include "model/DatasetLoader.h"

namespace ctb::app
{
   DatasetListView::DatasetListView(DatasetEventSourcePtr source) : m_sink{ this, source }
   {
   }

   void DatasetListView::notify([[maybe_unused]] DatasetEvent event)
   {
   }


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
      wnd->initControls();
      return wnd.release(); // parent owns child, so we don't need to delete
   }

   void DatasetListView::initControls()
   {
      auto model = m_sink.getTable();
      if (!model)
      {
         log::error("DatasetListView initialized with null dataset ptr, cannot continue");
         throw ctb::Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }
      AssociateModel(model.get());
      //AppendTextColumn("Wine", enumToIndex(WineListTraits::PropId::WineAndVintage));
      //AppendTextColumn("Region/Appellation", enumToIndex(WineListTraits::PropId::Locale));
   }

   void DatasetListView::setDataset()
   {
   }

} // namespace ctb::app