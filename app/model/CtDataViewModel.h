#include "App.h"

#include <ctb/interfaces/IDataset.h>
#include <wx/dataview.h>

namespace ctb::app
{
   class CtDataViewModel final : protected wxDataViewVirtualListModel
   {
   public:
      using base     = wxDataViewVirtualListModel;
      using ModelPtr = wxObjectDataPtr<CtDataViewModel>;

      /// @brief Returns a pointer to the active dataset (if any)
      /// @return the current dataset, may be nullptr/empty
      static [[nodiscard]] auto create(DatasetPtr dataset = {}) -> ModelPtr;


      auto getDataset() -> DatasetPtr;

      void setDataset(DatasetPtr dataset);

      /// @brief Forces a refresh of the dataview after large changes to underlying dataset
      void reQuery();

      void associateView(wxDataViewCtrl* view);

      // we only expose a limited amount of base-class API
      using base::GetItem;
      using base::GetRow;

      // these need to be accessible for wxObjectDataPtr to call, but users of this class
      // should just use ModelPtr so that ref counting is automatic.
      using base::IncRef;
      using base::DecRef;

   private:
      DatasetPtr m_dataset{};

      explicit CtDataViewModel(DatasetPtr dataset = {}) : m_dataset{ dataset }
      {}

      // these are the real purpose of this class, they're called by the base class to
      // provide data when the list-view needs it.
      void GetValueByRow(wxVariant& variant, unsigned row, unsigned col) const override;
      auto SetValueByRow(const wxVariant&, unsigned, unsigned) -> bool override;
      auto GetCount() const -> unsigned int override;
   };

   using DataViewModelPtr = CtDataViewModel::ModelPtr;

} // namespace ctb::app