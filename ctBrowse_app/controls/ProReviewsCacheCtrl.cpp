
#include "ProReviewsCacheCtrl.h"
#include <ctb/model/CtDatasetMgr.h>

namespace ctb::app
{

   auto ProReviewsCacheCtrl::create(wxWindow* parent, const DatasetEventSourcePtr& source, CacheValueFn value_fn) -> ProReviewsCacheCtrl*
   {
      return detail::createDatasetWindow<ProReviewsCacheCtrl>(parent, source, value_fn);
   }


   ProReviewsCacheCtrl::ProReviewsCacheCtrl(const DatasetEventSourcePtr& source, CacheValueFn value_fn)
      : ElasticPropertyValueBase{ source },
        m_value_fn{ std::move(value_fn) }
   {}

   auto ProReviewsCacheCtrl::getDisplayValue(const IDataset* ds) const -> std::string
   {
      std::string value{};

      auto& cache   = wxGetApp().getDatasetManager().getProReviewsCache();
      auto  wine_id = ds->getProperty(CtProp::iWineId).asUInt64().value_or(0);
      value         = (cache.*m_value_fn)(wine_id);

      return value;
   }


}   // namespace ctb::app
