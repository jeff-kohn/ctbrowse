#pragma once

#include "App.h"
#include "controls/ElasticPropertyValueBase.h"


namespace ctb::app
{

   /// @brief This class uses drink start/end properties to display a formatted drinking window.
   ///
   class DrinkWindowCtrl : public ElasticPropertyValueBase
   {
   public:
      /// @brief static factory method for creating DrinkWindowCtrl objects
      /// @param parent parent window for the control, must be non-null
      /// @param source dataset event source to bind the control to, must be non-null
      /// @param bound_prop the property id to bind the control to
      /// @return non-owning pointer to the newly created window.
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source, CtProp begin_prop, CtProp end_prop) -> DrinkWindowCtrl*
      {
         return detail::createDatasetWindow<DrinkWindowCtrl>(parent, source, begin_prop, end_prop);
      }

   private:
      CtProp m_begin_prop{};
      CtProp m_end_prop{};

      DECLARE_DATASET_WINDOW_FACTORY;

      DrinkWindowCtrl(const DatasetEventSourcePtr& source, CtProp begin_prop, CtProp end_prop)
         : ElasticPropertyValueBase{ source }, m_begin_prop{ begin_prop }, m_end_prop{ end_prop }
      {}

      auto getDisplayValue(const DatasetPtr& ds, int rec_idx) const -> std::string override
      {
         std::string value{};

         if (ds->hasProperty(m_end_prop))
         {
            auto begin_dt = ds->getProperty(rec_idx, m_begin_prop);
            auto end_dt   = ds->getProperty(rec_idx, m_end_prop);
            value         = ctb::detail::getDrinkWindow(begin_dt, end_dt);
         }
         return value;
      }

   };

}   // namespace ctb::app
