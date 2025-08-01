
#include "model/CtDataViewModel.h"

namespace ctb::app
{

   [[nodiscard]] auto CtDataViewModel::create(DatasetPtr dataset) -> ModelPtr
   {
      return ModelPtr{ new CtDataViewModel{ dataset } };
   }


   auto CtDataViewModel::getDataset() -> DatasetPtr 
   {
      return m_dataset;
   }


   void CtDataViewModel::setDataset(DatasetPtr dataset)
   {
      m_dataset = dataset;
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

#if !defined(NDEBUG)
      if ( row >= m_dataset->rowCount() or col >= std::ssize(m_dataset->listColumns()) )
      {
         SPDLOG_DEBUG("CtDataViewModel::GetValueByRow() called with invalid coordinates.");
         assert(false);
         return;
      }
#endif
      const auto& list_col = m_dataset->listColumns()[col];

      // format as string and return it to caller
      auto& val = m_dataset->getProperty(static_cast<int>(row), list_col.prop_id);
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
         return static_cast<uint32_t>(m_dataset->rowCount());

      return 0;
   }


} // namespace ctb::app