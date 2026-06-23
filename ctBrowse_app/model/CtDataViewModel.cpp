#include "CtDataViewModel.h"

#include <utility>


namespace ctb::app
{

   [[nodiscard]] auto CtDataViewModel::create(const IDataset* dataset) -> ModelPtr
   {
      return ModelPtr{ new CtDataViewModel{ dataset } };
   }


   auto CtDataViewModel::getDataset() -> const IDataset* 
   {
      return m_dataset;
   }


   void CtDataViewModel::setDataset(const IDataset* dataset)
   {
      m_dataset = std::move(dataset);
      reQuery();
   }


   void CtDataViewModel::reQuery()
   {
      Cleared();
   }


   void CtDataViewModel::associateView(wxDataViewCtrl* view) 
   {
      view->AssociateModel(this);
   }


   void CtDataViewModel::GetValueByRow(wxVariant& variant, unsigned row, unsigned col) const 
   {
      auto row_count = m_dataset->rowCount();
      auto col_count = std::ssize(m_dataset->availableListColumns());
      if ( std::cmp_greater_equal(row , row_count) or std::cmp_greater_equal(col , col_count))
      {
         SPDLOG_DEBUG("CtDataViewModel::GetValueByRow() called with invalid coordinates {} (max {}), {} (max{}).", row, row_count, col, col_count);
         return;
      }
      const auto& list_col = m_dataset->availableListColumns()[col];

      // format as string and return it to caller
      const auto& val = m_dataset->getRowProperty(row, list_col.prop_id);
      variant = list_col.getDisplayValue(val);
   }


   auto CtDataViewModel::SetValueByRow(const wxVariant&, unsigned, unsigned) -> bool 
   {
      return false; // editing not supported
   }


   unsigned int CtDataViewModel::GetCount()	const 
   {
      // this may get by base class (via event handler) when our dataset is null because we received a DatasetRemoved() event.
      if (m_dataset)
         return m_dataset->rowCount();

      return 0;
   }


} // namespace ctb::app
