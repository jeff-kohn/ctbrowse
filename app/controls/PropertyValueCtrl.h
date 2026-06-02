#pragma once

#include "App.h"
#include "controls/ElasticPropertyValueBase.h"


namespace ctb::app
{

   /// @brief standard/default property control that displays a single property
   ///
   class PropertyValueCtrl : public ElasticPropertyValueBase
   {
   public:
      // clang-format off
      /// @brief static factory method for creating PropertyValueCtrl objects
      /// @param parent parent window for the control, must be non-null
      /// @param source dataset event source to bind the control to, must be non-null
      /// @param bound_prop the property id to bind the control to
      /// @return non-owning pointer to the newly created window.
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source, CtProp bound_prop) -> PropertyValueCtrl*
      {
         return detail::createDatasetWindow<PropertyValueCtrl>(parent, source, bound_prop);
      }

      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source, CtProp bound_prop,
                                       std::string_view format, std::string_view null_display) -> PropertyValueCtrl*
      {
         auto* ctrl = detail::createDatasetWindow<PropertyValueCtrl>(parent, source, bound_prop);

         ctrl->setNullDisplayValue(null_display);
         if (!format.empty())
         {
            ctrl->setFormat(format);
         }
         return ctrl;
      }
      // clang-format on


      /// @brief Set the display format.
      ///
      /// Default is "{}" which just displays the string property, but you can change it if needed (e.g. currency etc)
      void setFormat(std::string_view fmt_str)
      {
         m_format_str = fmt_str;
      }

      /// @brief Set the value to display when the bound field isNull(). Default is empty string
      void setNullDisplayValue(std::string_view val)
      {
         m_null_display = val;
      }

   private:
      CtProp      m_prop{};
      std::string m_format_str{ constants::FMT_DEFAULT_FORMAT };
      std::string m_null_display{};

      DECLARE_DATASET_WINDOW_FACTORY;

      PropertyValueCtrl(const DatasetEventSourcePtr& source, CtProp bound_prop) : ElasticPropertyValueBase{ source }, m_prop{ bound_prop }
      {}

      auto getDisplayValue(const DatasetPtr& ds, int rec_idx) const -> std::string override
      {
         auto val = ds->getProperty(rec_idx, m_prop);

         if (val.isNull()) return m_null_display;

         return val.asString(m_format_str);
      }
   };

}
