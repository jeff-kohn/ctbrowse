#pragma once

#include "App.h"
#include "controls/ElasticPropertyValueBase.h"


namespace ctb::app
{

   /// @brief This class binds a property value control to one of the getXXX() methods in the ProReviewsCache class.
   ///
   class ProReviewsCacheCtrl final : public ElasticPropertyValueBase
   {
   public:
      using CacheValueFn = std::string (ProReviewsCache::*)(uint64_t) const;

      /// @brief static factory method for creating ProReviewsCacheCtrl objects
      /// @param parent     - the parent window for the control, must be non-null
      /// @param source     - the dataset event source to bind the control to, must be non-null
      /// @param bound_prop - the property id to bind the control to
      /// @return non-owning pointer to the newly created window.
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source, CacheValueFn value_fn) -> ProReviewsCacheCtrl*
      {
         return detail::createDatasetWindow<ProReviewsCacheCtrl>(parent, source, value_fn);
      }
   private:
      CacheValueFn m_value_fn{};

      ProReviewsCacheCtrl(const DatasetEventSourcePtr& source, CacheValueFn value_fn)
         : ElasticPropertyValueBase{ source },
           m_value_fn{ std::move(value_fn) }
      {}

      auto getDisplayValue(const IDataset* ds) const -> std::string override
      {
         std::string value{};

         auto& cache   = wxGetApp().getDatasetManager().getProReviewsCache();
         auto  wine_id = ds->getProperty(CtProp::iWineId).asUInt64().value_or(0);
         value         = (cache.*m_value_fn)(wine_id);

         return value;
      }

      DECLARE_DATASET_WINDOW_FACTORY;
   };

}   // namespace ctb::app
