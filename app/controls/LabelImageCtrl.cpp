#include "LabelImageCtrl.h"


#include <wx/sizer.h>


namespace ctb::constants
{
   constexpr auto LABEL_TIMER_RETRY_INTERVAL = 33;

} // namespace ctb::constants


namespace ctb::app
{
   auto LabelImageCtrl::create(wxWindow* parent, const DatasetEventSourcePtr& source) -> LabelImageCtrl*
   {
      if (!parent or !parent->GetSizer())
      {
         assert("parent window and sizer cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }
      if (!source)
      {
         assert("source parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }

      // non-null parent owns the new object, so raw ptr is ok.
      return new LabelImageCtrl{ parent, source };
   }


   LabelImageCtrl::LabelImageCtrl(wxWindow* parent, const DatasetEventSourcePtr& source) :
      wxGenericStaticBitmap{ parent, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE },
      m_cache { wxGetApp().getLabelCache() },
      m_event_handler{ source },
      m_parent_sizer{ parent->GetSizer() }
   {
      assert(m_cache);
      initControls();
   }


   void LabelImageCtrl::initControls()
   {
      SetScaleMode(wxStaticBitmap::Scale_AspectFit);

      // hook up event handlers
      m_label_timer.Bind(wxEVT_TIMER, &LabelImageCtrl::onLabelTimer, this);
      m_event_handler.addHandler(DatasetEvent::Id::RowSelected, [this](const DatasetEvent& event) { fetchImage(event); });
   }


   void LabelImageCtrl::checkLabelResult()
   {
      using namespace tasks;

      if (auto& result = m_image_result)
      {
         switch (result->poll(0ms))
         {
         case wxImageTask::Status::Deferred: [[fallthrough]];
         case wxImageTask::Status::Finished:
            displayLabel();
            [[fallthrough]];

         case wxImageTask::Status::Invalid:
            m_image_result = {};
            break;

         case wxImageTask::Status::Running:
            m_label_timer.StartOnce(constants::LABEL_TIMER_RETRY_INTERVAL);
            break;

         default:
            assert("Bug, new enum value wasn't accounted for" and false);
            break;
         }
      }
   }


   void LabelImageCtrl::displayLabel()
   {
      try
      {
         if (m_image_result)
         {
            auto result = m_image_result->getImage();
            if (!result)
               // the move is necessary because expected::error() returns a reference that would immediately go out of scope.
               throw Error{ std::move(result.error()) }; 

            wxBitmap bmp{ *result };
            SetBitmap(bmp);
            Show();
            GetParent()->Layout(); // required since the images vary in size
            GetParent()->SendSizeEvent();
         }
      }
      catch (...)
      {
         log::exception(packageError());
         SetBitmap(wxBitmap{});
         Hide();
         Refresh();
         Update();
      }
   }


   void LabelImageCtrl::onLabelTimer(wxTimerEvent&)
   {
      checkLabelResult();
   }


   void LabelImageCtrl::fetchImage(const DatasetEvent& event)
   {
      // we always hide it, it will be shown once we successfully retrieve the image
      Hide();
      if (event.dataset && event.affected_row.has_value())
      {
         auto wine_id = event.dataset->getProperty(event.affected_row.value(), CtProp::iWineId).asUInt64().value_or(0);
         m_image_result = m_cache->fetchLabelImage(wine_id);
         checkLabelResult();
      }
   }

} // namespace ctb::app