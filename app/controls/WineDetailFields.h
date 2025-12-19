#pragma once

#include <ctb/interfaces/IDataset.h>
#include <ctb/tables/detail/field_helpers.h>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/valgen.h>

#include <variant>


namespace ctb::app
{
   namespace detail
   {

      class DetailFieldControls
      {
      public:
         static constexpr auto COL_COUNT = 2;

         DetailFieldControls(wxSizer* parent_sizer, std::string_view heading_label) : m_parent_sizer{ parent_sizer }
         {
            auto* parent_wnd = m_parent_sizer ? m_parent_sizer->GetContainingWindow() : nullptr;
            if (!parent_wnd)
               throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };

            m_label_wnd = new wxStaticText{ parent_wnd, wxID_ANY, wxFromSV(heading_label) };
            m_value_wnd = new wxStaticText{ parent_wnd, wxID_ANY, wxEmptyString           };
            m_value_wnd->SetValidator(wxGenericValidator{ m_display_value.get() });

            m_row_sizer = new wxGridSizer{ COL_COUNT };
            m_row_sizer->Add(m_label_wnd, wxSizerFlags{}.Expand().Border(wxLEFT | wxRIGHT).Right());
            m_row_sizer->Add(m_value_wnd, wxSizerFlags{}.Expand().Border(wxLEFT | wxRIGHT));
            parent_sizer->Add(m_row_sizer, wxSizerFlags{}.CenterHorizontal());

            m_created = true;
         }

         void show()
         {
            m_parent_sizer->Show(m_row_sizer, true, true);
         }

         void hide()
         {
            m_parent_sizer->Show(m_row_sizer, false, true);
         }

         void setValue(std::string_view value_str)
         {
            *m_display_value = wxFromSV(value_str);
         }

      private:
         bool          m_created{ false };
         wxSizer*      m_parent_sizer{};
         wxSizer*      m_row_sizer{};
         wxStaticText* m_label_wnd{};
         wxStaticText* m_value_wnd{};

         // we need the address of the wxString to be stable for the validator, which stores a ptr. 
         std::unique_ptr<wxString> m_display_value{ new wxString{} };
      };

   }


   class SinglePropDetailField
   {
   public:
      SinglePropDetailField() = delete;
      SinglePropDetailField(wxSizer* parent_sizer, CtProp prop_id, std::string_view label_text) : 
         m_controls{ parent_sizer, label_text },
         m_prop_id{ prop_id }
      {}

      void clear()
      {
         m_controls.setValue("");
         m_controls.hide();
      }

      /// @brief update the field values from the specified dataset row
      void update(const DatasetPtr& ds, int rec_idx)
      {
         if (ds->hasProperty(m_prop_id))
         {
            auto val = ds->getProperty(rec_idx, m_prop_id);
            m_controls.setValue(val.hasValue() ? val.asString(m_format_str) : m_null_display);
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
      template<typename Self>
      auto setFormat(this Self&& self, std::string_view fmt_str) 
      {
         self.m_format_str = fmt_str;
         return std::forward<Self>(self);
      }

      /// @brief Set the value to display when the bound field isNull(). Default is empty string
      template<typename Self>
      auto setNullDisplayValue(this Self&& self, std::string_view val) 
      {
         self.m_null_display = val;
         return std::forward<Self>(self);
      }

   private:
      detail::DetailFieldControls m_controls;
      CtProp                      m_prop_id;
      std::string                 m_format_str{ constants::FMT_DEFAULT_FORMAT };
      std::string                 m_label_text{};
      std::string                 m_null_display{};
   };


   class DrinkWindowDetailField
   {
   public:
      DrinkWindowDetailField() = delete;
      DrinkWindowDetailField(wxSizer* parent_sizer, CtProp begin_prop, CtProp end_prop, std::string_view label_text) :
         m_controls{ parent_sizer, label_text },
         m_begin_prop{ begin_prop },
         m_end_prop{ end_prop }
      {}

      void clear()
      {
         m_controls.setValue("");
         m_controls.hide();
      }

      void update(const DatasetPtr& ds, int rec_idx)
      {
         if (ds->hasProperty(m_end_prop))
         {
            auto begin_dt = ds->getProperty(rec_idx, m_begin_prop);
            auto end_dt   = ds->getProperty(rec_idx, m_end_prop);
            m_controls.setValue(ctb::detail::getDrinkWindow(begin_dt, end_dt));
            m_controls.show();
         }
         else {
            m_controls.setValue("");
            m_controls.hide();
         }
      }
      
   private:
      detail::DetailFieldControls m_controls;
      CtProp                      m_begin_prop{};
      CtProp                      m_end_prop{};
   };

   using WineDetailsField  = std::variant<SinglePropDetailField, DrinkWindowDetailField>;
   using WineDetailsFields = std::deque<WineDetailsField>;

} // namespace ctb::app 
