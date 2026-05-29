#pragma once

#include "App.h"

#include <wx/textctrl.h>
#include <wx/valgen.h>

namespace ctb::app
{


   // single-line, readonly text control that automatically resizes to fit its text
   //
   class ElasticTextCtrl : public wxTextCtrl
   {
   public:
      static auto create(wxWindow* parent) -> ElasticTextCtrl*
      {
         if (!parent)
         {
            assert("parent parameter cannot == nullptr" and false);
            throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
         }
         return new ElasticTextCtrl{ parent };
      }

      void setValue(std::string_view value_str)
      {
         m_display_value = wxFromSV(value_str);

         if (m_display_value.empty()) return;

         // text controls don't auto-expand to fit text the way static controls do, have to force it.
         constexpr auto select_margin = 3;
         auto           sz            = GetSizeFromText(m_display_value);
         sz.SetWidth(sz.GetWidth() + select_margin);   // a little wiggle room to prevent horizontal scrolling when selecting text.
         InvalidateBestSize();
         SetMinClientSize(sz);
      }

      ~ElasticTextCtrl() noexcept override               = default;

      ElasticTextCtrl(ElasticTextCtrl&&)                 = delete;
      ElasticTextCtrl(const ElasticTextCtrl&)            = delete;
      ElasticTextCtrl& operator=(ElasticTextCtrl&&)      = delete;
      ElasticTextCtrl& operator=(const ElasticTextCtrl&) = delete;

   private:
      wxString m_display_value{};

      ElasticTextCtrl(wxWindow* parent) : wxTextCtrl{ parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxBORDER_NONE }
      {
         SetValidator(wxGenericValidator{ &m_display_value });
      }
   };

}   // namespace ctb::app
