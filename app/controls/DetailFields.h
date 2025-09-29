#pragma once

#include <ctb/tables/detail/field_helpers.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/valgen.h>

namespace ctb::app
{
   class DetailFieldControls
   {
   public:
      DetailFieldControls(wxWindow& parent, wxGridSizer& sizer, std::string_view label) : 
         m_sizer{ &sizer },
         m_label_wnd{ new wxStaticText{ &parent, wxID_ANY, wxFromSV(label) }},
         m_value_wnd{ new wxStaticText{ &parent, wxID_ANY, wxEmptyString   }}
      {
         m_sizer->Add(m_label_wnd, wxSizerFlags{}.Border(wxLEFT | wxRIGHT).Right());
         m_sizer->Add(m_value_wnd, wxSizerFlags{}.Border(wxLEFT | wxRIGHT));
         m_value_wnd->SetValidator(wxGenericValidator{ &m_display_value });
      }

      void show()
      {
         m_sizer->Hide(m_label_wnd);
         m_sizer->Hide(m_value_wnd);
      }

      void hide()
      {
         m_sizer->Show(m_label_wnd);
         m_sizer->Show(m_value_wnd);
      }

      void setValue(std::string_view value_str)
      {
         m_display_value = wxFromSV(value_str);
      }

      DetailFieldControls() = delete;
      //DetailFieldControls(const DetailFieldControls&) = default;
      //DetailFieldControls(DetailFieldControls&&) = default;
      //DetailFieldControls& operator=(const DetailFieldControls&) = default;
      //DetailFieldControls& operator=(DetailFieldControls&&) = default;
      //~DetailFieldControls() = default;

   private:
      wxSizer      * m_sizer{};
      wxStaticText * m_label_wnd{};
      wxStaticText * m_value_wnd{};
      wxString       m_display_value{};
   };


   class SinglePropDetailField
   {
   public:
      SinglePropDetailField(wxWindow& parent, wxGridSizer& sizer, CtProp prop_id, std::string_view label);

      void updateField(const DatasetPtr& ds, int rec_idx)
      {
         if (ds->hasProperty(m_prop_id))
         {
            m_controls.setValue(ds->getProperty(rec_idx, m_prop_id).asString(m_format_str));
            m_controls.show();
         }
         else {
            m_controls.setValue("");
            m_controls.hide();
         }
      }

      /// @brief Set the display format. 
      /// 
      /// Default is "{}" which just displays the string property, but you can change it if needed (e.g. currency etc)
      auto setFormat(std::string_view fmt_str) -> SinglePropDetailField&
      {
         m_format_str = fmt_str;
         return *this;
      }

      SinglePropDetailField() = delete;
      //SinglePropDetailField(const SinglePropDetailField&) = default;
      //SinglePropDetailField(SinglePropDetailField&&) = default;
      //SinglePropDetailField& operator=(const SinglePropDetailField&) = default;
      //SinglePropDetailField& operator=(SinglePropDetailField&&) = default;
      //~SinglePropDetailField() = default;

   private:
      DetailFieldControls m_controls;
      CtProp              m_prop_id;
      std::string         m_format_str{ constants::FMT_DEFAULT_FORMAT };
   };


   class DrinkWindowDetailField
   {
   public:
      DrinkWindowDetailField(wxWindow& parent, wxGridSizer& sizer, CtProp begin_prop, CtProp end_prop, std::string_view label);

      void updateField(const DatasetPtr& ds, int rec_idx)
      {
         if (ds->hasProperty(m_end_prop))
         {
            auto begin_dt = ds->getProperty(rec_idx, m_begin_prop);
            auto end_dt   = ds->getProperty(rec_idx, m_end_prop);
            m_controls.setValue(detail::getDrinkWindow(begin_dt, end_dt));
            m_controls.show();
         }
         else {
            m_controls.setValue("");
            m_controls.hide();
         }
      }

      DrinkWindowDetailField() = delete;
      //DrinkWindowDetailField(const DrinkWindowDetailField&) = default;
      //DrinkWindowDetailField(DrinkWindowDetailField&&) = default;
      //DrinkWindowDetailField& operator=(const DrinkWindowDetailField&) = default;
      //DrinkWindowDetailField& operator=(DrinkWindowDetailField&&) = default;
      //~DrinkWindowDetailField() = default;   
      
   private:
      DetailFieldControls m_controls;
      CtProp              m_begin_prop;
      CtProp              m_end_prop;
   };
}