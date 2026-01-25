#include "views/DatasetMultiView.h"
#include "views/DetailsViewMyCellar.h"
#include "views/DetailsViewPending.h"
#include "views/DetailsViewReadyToDrink.h"
#include "views/DatasetListView.h"
#include "views/DatasetOptionsView.h"

#include <wx/persist/splitter.h>


namespace ctb::app
{


   [[nodiscard]]
   auto DatasetMultiView::create(wxWindow* parent, const DatasetEventSourcePtr& source) -> DatasetMultiView*
   {
      try
      {
         if (!parent)
         {
            assert("parent parameter cannot == nullptr");
            throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
         }
         
         // object is owned by its (guaranteed) non-nullptr parent, so we're returning raw non-nowning ptr.
         return new DatasetMultiView{ parent, source };
      }
      catch (...){
         log::exception(packageError());
         throw;
      }
   }


   namespace
   {
      using DetailsViewFactory = std::function<DetailsViewBase*(wxWindow* parent, const DatasetEventSourcePtr& source)>;
   
      static auto details_view_map = std::map<TableId, DetailsViewFactory>
      {
         { TableId::List,         &DetailsViewMyCellar::create       } ,
         { TableId::Pending,      &DetailsViewPending::create        } ,
         { TableId::Availability, &DetailsViewReadyToDrink::create   } ,
      };

      /// @brief Creates and returns the appropriate DetailsViewBase-derived window object for the event source's dataset 
      /// @return A non-owning pointer to the newly created details view for the dataset.
      /// @throw ctb::Error if any required parameters are null or there is no factory for the dataset type.
      auto createDetailsView(wxWindow* parent, const DatasetEventSourcePtr& source) -> DetailsViewBase*
      {
         if (!source or !source->hasDataset())
         {
            throw Error{ constants::ERROR_STR_DETAILS_VIEW_NULL_DATASET, Error::Category::ArgumentError };
         }

         const auto table_id = source->getDataset()->getTableId();
         const auto it = details_view_map.find(table_id);
         if (it != details_view_map.end())
         {
            return it->second(parent, source);
         }
         else
         {
            throw Error{ ctb::format(constants::FMT_ERROR_STR_INVALID_DETAIL_DETAIL, magic_enum::enum_name(table_id)), Error::Category::ArgumentError };
         }
      }

   } // namespace

   
   DatasetMultiView::DatasetMultiView(wxWindow* parent, const DatasetEventSourcePtr& source) : wxSplitterWindow{ parent }
   {
      constexpr auto LEFT_SPLITTER_GRAVITY = 0.25;
      constexpr auto RIGHT_SPLITTER_GRAVITY = 0.75;
      constexpr auto MIN_PANE_SIZE = 100;

      auto font = GetFont();
      font.SetPointSize(font.GetPointSize() + 1);
      SetFont(font);

      SetSashGravity(LEFT_SPLITTER_GRAVITY);

      // this splitter window contains options panel and right/nested splitter
      m_options_panel = DatasetOptionsView::create(this, source);
      m_right_splitter = new wxSplitterWindow{ this };
      SplitVertically(m_options_panel, m_right_splitter);
      wxPersistentRegisterAndRestore(this, "DatasetMultiView");

      // nested splitter contains grid and details
      m_listView = DatasetListView::create(m_right_splitter, source);
      m_details_panel = createDetailsView(m_right_splitter, source);
      m_right_splitter->SplitVertically(m_listView, m_details_panel);
      m_right_splitter->SetMinimumPaneSize(MIN_PANE_SIZE);
      wxPersistentRegisterAndRestore(m_right_splitter, "DatasetMultiViewNested");
      
      // For some reason the CalLAfter() is required, otherwise this call messes up the
      // next splitter layout. No idea why but this works.
      CallAfter([this] { m_right_splitter->SetSashGravity(RIGHT_SPLITTER_GRAVITY); });
   }


} // namespace ctb::app
