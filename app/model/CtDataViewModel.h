#include "App.h"

#include <ctb/model/DatasetBase.h>
#include <wx/dataview.h>

namespace ctb::app
{
   class CtDataViewModel final : public wxDataViewVirtualListModel
   {
   public:
      CtDataViewModel() = default;
      explicit CtDataViewModel(DatasetPtr dataset) : m_dataset{ dataset }
      {}

      void setDataset(DatasetPtr dataset)
      {
         m_dataset = dataset;
      }

      template<typename Self>
      auto getDataset(this Self&& self) -> DatasetPtr 
      {
         return std::forward<Self>(self).m_dataset;
      }

      void GetValueByRow(wxVariant& variant, unsigned row, unsigned col) const override
      {

#if !defined(NDEBUG)
         if ( row >= m_dataset->filteredRowCount() or col >= std::ssize(m_dataset->displayColumns()) )
         {
            assert(false);
            SPDLOG_DEBUG("CtDataViewModel::GetValueByRow() called with invalid coordinates.");
            return;
         }
#endif

         auto display_col = m_dataset->displayColumns().at(col);
         auto prop = display_col.prop_index;

         // format as string and return it to caller
         auto val = m_dataset->getDetailProp(static_cast<int>(row), prop);
         auto val_str = display_col.getDisplayValue(val);
         variant = wxString::FromUTF8(val_str);
      }

      bool SetValueByRow(const wxVariant&, unsigned, unsigned) override
      {
         return false; // editing not supported
      }


      unsigned int GetCount()	const override
      {
         return static_cast<uint32_t>(m_dataset->filteredRowCount());
      }

   private:
      DatasetPtr m_dataset{};
   };

} // namespace ctb::app