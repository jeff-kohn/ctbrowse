#include "LabelImageCtrl.h"

#include <ctb/model/CtDatasetMgr.h>
#include <wx/mstream.h>
#include <wx/sizer.h>


namespace ctb::app
{

   void LabelImageCtrl::FetchLabelCallback::operator()(ImageResult result) const
   {
      if (!m_wnd) return;

      m_wnd->CallAfter(
         [wnd = m_wnd, result = std::move(result)]
         {
            if (!wnd) return;
            try
            {
               if (!result) throw Error{ result.error() };

               wxMemoryInputStream byte_stream(result->contents.data(), result->contents.size());
               wxImage             label_img{};
               label_img.LoadFile(byte_stream, wxBITMAP_TYPE_JPEG);
               wnd->labelUpdate(result->wine_id, label_img);
            }
            catch (...)
            {
               log::exception(packageError());
            }
         });
   }


   auto LabelImageCtrl::create(wxWindow* parent, const DatasetEventSourcePtr& source) -> LabelImageCtrl*
   {
      return detail::createDatasetWindow<LabelImageCtrl>(parent, source);
   }


   LabelImageCtrl::LabelImageCtrl(const DatasetEventSourcePtr& source) : Base{ source }
   {}


   void LabelImageCtrl::createWindow(wxWindow* parent)
   {
      if (!Create(parent, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }
      SetScaleMode(wxStaticBitmap::Scale_AspectFit);

      // hook up event handlers
      getEventHandler().addHandler(DatasetEvent::Id::RowSelected,
                                   [this](const DatasetEvent& event)
                                   {
                                      fetchImage(event);
                                   });
   }


   void LabelImageCtrl::labelUpdate(uint64_t wine_id, wxImage& image_result)
   {
      if (wine_id != m_current_wine_id) return;

      try
      {
         wxBitmap bmp{ image_result };
         SetBitmap(bmp);
         Show();
         GetParent()->Layout();   // required since the images vary in size
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


void LabelImageCtrl::fetchImage(const DatasetEvent& event)
{
   // we always hide it, it will be shown once we successfully retrieve the image
   Hide();
   if (event.dataset && event.affected_row.has_value())
   {
      m_current_wine_id = event.dataset->getProperty(CtProp::iWineId).asUInt64().value_or(0);
      wxGetApp().getDatasetManager().retrieveLabelImageAsync(m_current_wine_id, FetchLabelCallback(this));
   }
}

}   // namespace ctb::app
