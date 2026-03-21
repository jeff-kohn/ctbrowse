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
      return detail::createDatasetWindow<LabelImageCtrl>(parent, source, wxGetApp().getLabelCache() );
   }


   LabelImageCtrl::LabelImageCtrl(const DatasetEventSourcePtr& source, LabelCachePtr cache) :
      Base { source },
      m_cache { std::move(cache) }
   {
      assert(m_cache);
   }

   void LabelImageCtrl::createWindow(wxWindow* parent)
   {
      if (!Create(parent, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }

      SetScaleMode(wxStaticBitmap::Scale_AspectFit);

      // hook up event handlers
      m_label_timer.Bind(wxEVT_TIMER, &LabelImageCtrl::onLabelTimer, this);
      getEventHandler().addHandler(DatasetEvent::Id::RowSelected, [this](const DatasetEvent& event) { fetchImage(event); });
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
               throw Error{ std::move(result.error()) }; 

            wxBitmap bmp{ *result };
            SetBitmap(bmp);
            Show();
            GetParent()->Layout(); // required since the images vary in size
            //GetParent()->SendSizeEvent();
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

         if (auto fetch_result = m_cache->fetchLabelImage(wine_id); fetch_result.has_value())
         {
            m_image_result = fetch_result.value();
         }
         else {
            m_image_result = std::nullopt;
            log::exception(fetch_result.error());
         }
         checkLabelResult();
      }
   }

} // namespace ctb::app