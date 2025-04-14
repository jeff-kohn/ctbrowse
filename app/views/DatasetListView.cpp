#include "views/DatasetListView.h"
#include "model/CellarTrackerDataModel.h"
#include "model/DatasetLoader.h"

namespace ctb::app
{
   CellarTrackerListView::CellarTrackerListView(DatasetEventSourcePtr source) : m_sink{ this, source }
   {
   }

   void CellarTrackerListView::notify([[maybe_unused]] DatasetEvent event)
   {
   }


   [[nodiscard]] CellarTrackerListView* CellarTrackerListView::create(wxWindow* parent, DatasetEventSourcePtr source)
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

      std::unique_ptr<CellarTrackerListView> wnd{ new CellarTrackerListView{source} };
      if (!wnd->Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }
      wnd->initControls();
      return wnd.release(); // parent owns child, so we don't need to delete
   }

   void CellarTrackerListView::initControls()
   {
      auto model = m_sink.getTable();
      if (!model)
      {
         log::error("CellarTrackerListView initialized with null dataset ptr, cannot continue");
         throw ctb::Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }
      AssociateModel(model.get());
      //AppendTextColumn("Wine", enumToIndex(WineListTraits::PropId::WineAndVintage));
      //AppendTextColumn("Region/Appellation", enumToIndex(WineListTraits::PropId::Locale));
   }

} // namespace ctb::app