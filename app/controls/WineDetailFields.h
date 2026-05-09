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
		
      class DisplayValue 
      {
      public:
         static constexpr auto COL_COUNT = 2;

         DisplayValue(wxSizer* parent_sizer, std::string_view heading_label) : m_parent_sizer{ parent_sizer }
         {
            auto* parent_wnd = m_parent_sizer ? m_parent_sizer->GetContainingWindow() : nullptr;
            if (!parent_wnd)
               throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };

            m_label_wnd = new wxStaticText{ parent_wnd, wxID_ANY, wxFromSV(heading_label) };    // cppcheck-suppress noOperatorEq
            m_value_wnd = new wxStaticText{ parent_wnd, wxID_ANY, wxEmptyString           };    // cppcheck-suppress noOperatorEq
            m_value_wnd->SetValidator(wxGenericValidator{ m_display_value.get() });

            m_row_sizer = new wxGridSizer{ COL_COUNT };                                         // cppcheck-suppress noOperatorEq
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
            if (m_display_value->Contains("&"))
            {
               m_display_value->Replace("&", "&&");
            }
         }

         DisplayValue(const DisplayValue&) = delete;
	      DisplayValue& operator=(const DisplayValue&) = delete;
         DisplayValue(DisplayValue&&) = default;
         DisplayValue& operator=(DisplayValue&&) = default;

      private:
         bool          m_created{ false };
         wxSizer*      m_parent_sizer{};
         wxSizer*      m_row_sizer{};
         wxStaticText* m_label_wnd{};
         wxStaticText* m_value_wnd{};

         // we need the address of the wxString to be stable for the validator, which stores a ptr. 
         std::unique_ptr<wxString> m_display_value{ new wxString{} };
      };

   } // namespace detail


   class SinglePropertyDisplay
   {
   public:
      SinglePropertyDisplay() = delete;
      SinglePropertyDisplay(wxSizer* parent_sizer, CtProp prop_id, std::string_view label_text) : 
         m_display_prop{ parent_sizer, label_text },
         m_prop_id{ prop_id }
      {}

      void clear()
      {
         m_display_prop.setValue("");
         m_display_prop.hide();
      }

      /// @brief update the field values from the specified dataset row
      void update(const DatasetPtr& ds, int rec_idx)
      {
         if (ds->hasProperty(m_prop_id))
         {
            auto val = ds->getProperty(rec_idx, m_prop_id);
            m_display_prop.setValue(val.hasValue() ? val.asString(m_format_str) : m_null_display);
            m_display_prop.show();
         }
         else {
            m_display_prop.setValue("");
            m_display_prop.hide();
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
      detail::DisplayValue    m_display_prop;
      CtProp                  m_prop_id;
      std::string             m_format_str{ constants::FMT_DEFAULT_FORMAT };
      std::string             m_label_text{};
      std::string             m_null_display{};
   };


   class DrinkWindowDisplay
   {
   public:
      DrinkWindowDisplay() = delete;
      DrinkWindowDisplay(wxSizer* parent_sizer, CtProp begin_prop, CtProp end_prop, std::string_view label_text) :
         m_display_prop{ parent_sizer, label_text },
         m_begin_prop{ begin_prop },
         m_end_prop{ end_prop }
      {}

      void clear()
      {
         m_display_prop.setValue("");
         m_display_prop.hide();
      }

      void update(const DatasetPtr& ds, int rec_idx)
      {
         if (ds->hasProperty(m_end_prop))
         {
            auto begin_dt = ds->getProperty(rec_idx, m_begin_prop);
            auto end_dt   = ds->getProperty(rec_idx, m_end_prop);
            m_display_prop.setValue(ctb::detail::getDrinkWindow(begin_dt, end_dt));
            m_display_prop.show();
         }
         else {
            m_display_prop.setValue("");
            m_display_prop.hide();
         }
      }
      
   private:
      detail::DisplayValue m_display_prop;
      CtProp                  m_begin_prop{};
      CtProp                  m_end_prop{};
   };


   class ProReviewDisplay
   {
   public:
      using CacheValueFn = std::string(ProReviewsCache::*)(uint64_t) const;

      ProReviewDisplay() = delete;
      ProReviewDisplay(wxSizer* parent_sizer, std::string_view label_text, CacheValueFn value_fn) :
         m_display_prop{ parent_sizer, label_text },
         m_value_fn{ std::move(value_fn) }
      {}

      /// @brief update the field values from the specified dataset row
      void update(const DatasetPtr& ds, int rec_idx)
      {
         auto wine_id = ds->getProperty(rec_idx, CtProp::iWineId).asUInt64().value_or(0);
         std::string value{};

         auto cache = wxGetApp().getProReviewsCache();
         if (cache)
         {
            value = (*cache.*m_value_fn)(wine_id);
         }

         if (value.empty())
         {
            m_display_prop.setValue("");
            m_display_prop.hide();
         }
         else {
            m_display_prop.setValue(value);
            m_display_prop.show();
         }
      }

      void clear()
      {
         m_display_prop.setValue("");
         m_display_prop.hide();
      }

   protected:
      detail::DisplayValue m_display_prop;
      CacheValueFn         m_value_fn{};
   };

} // namespace ctb::app 
