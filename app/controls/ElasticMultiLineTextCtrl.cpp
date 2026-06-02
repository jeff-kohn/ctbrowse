#include "ElasticMultiLineTextCtrl.h"

#include <wx/valgen.h>

namespace ctb::app
{

   [[nodiscard]] auto
   ElasticMultiLineTextCtrl::create(wxWindow* parent, const DatasetEventSourcePtr& source, CtProp bound_prop, wxAlignment align)
      -> ElasticMultiLineTextCtrl*
   {
      return detail::createDatasetWindow<ElasticMultiLineTextCtrl>(parent, source, bound_prop, align);
   }


   void ElasticMultiLineTextCtrl::createWindow(wxWindow* parent)
   {
      assert(parent);

      const auto styles = wxTE_MULTILINE | wxTE_BESTWRAP | wxTE_READONLY | wxTE_NO_VSCROLL | wxBORDER_NONE | m_align_flag;
      if (!wxTextCtrl::Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, styles))
      {
         throw Error{Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED};
      }
      SetBackgroundColour(parent->GetBackgroundColour());
      SetValidator(wxGenericValidator{ &m_display_value });

      Bind(wxEVT_SIZE, &ElasticMultiLineTextCtrl::onSize, this);
      getEventHandler().addHandler(DatasetEventHandler::EventId::RowSelected, [this](const DatasetEvent& event) { onDatasetRowSelected(event); });
   }


   void ElasticMultiLineTextCtrl::onSize([[maybe_unused]] wxSizeEvent& event)
   {
      if (m_need_resize)
      {
         // Lock updates to prevent cascading size events
         wxWindowUpdateLocker lock(this);

         auto text_size   = calcTextSize();
         auto client_size = GetClientSize();
         if (text_size.GetHeight() > client_size.GetHeight())
         {
            SetWindowStyleFlag(GetWindowStyle() & ~wxTE_NO_VSCROLL);
         }
         else
         {
            SetWindowStyleFlag(GetWindowStyle() | wxTE_NO_VSCROLL);
         }
         m_need_resize = false;
      }
      event.Skip();
   }


   void ElasticMultiLineTextCtrl::onDatasetRowSelected(const DatasetEvent& event)
   {
      assert(event.dataset);
      if (!event.affected_row.has_value()) return;

      auto val = event.dataset->getProperty(event.affected_row.value(), m_prop);
      m_display_value = val.hasString() ? wxFromSV(val.asStringView()) : val.asString();
      TransferDataToWindow();

      InvalidateBestSize();
      SetMinClientSize(calcTextSize());
      m_need_resize = true;
   }


   auto ElasticMultiLineTextCtrl::calcTextSize() -> wxSize
   {
      // calculate how wide our note control can be and still fit in panel, allowing for sizer borders.
      constexpr auto margin = 30;
      const auto max_width  = GetClientSize().GetWidth() - margin;

      // Calculate height based on number of lines
      auto num_lines    = GetNumberOfLines();
      auto line_height  = GetCharHeight();

      return wxSize{ max_width, num_lines * line_height };
   }
}

