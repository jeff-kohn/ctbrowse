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
      [[nodiscard]] static auto create() -> ModelPtr;


      /// @brief Returns the active dataset for this model (may be empty/null)
      auto getDataset() -> DatasetPtr;

      /// @brief Forces a refresh of the data view after large changes to underlying dataset
      void reQuery();

      /// @brief associate a data view ctrl with this model. we only support one associated view, last call wins.
      void associateView(wxDataViewCtrl* view);

      // we only expose a limited amount of base-class API
      using base::GetItem;
      using base::GetRow;

      // these need to be accessible for wxObjectDataPtr to call, but users of this class
      // should just use ModelPtr so that ref counting is automatic.
      using base::IncRef;
      using base::DecRef;

   private:
      DatasetPtr       m_dataset{};
      wxDataViewCtrl*  m_view{};

      explicit CtDataViewModel(DatasetEventSourcePtr source)
      {}

      // these are the real purpose of this class, they're called by the base class to
      // provide data when the list-view needs it.
      void GetValueByRow(wxVariant& variant, unsigned row, unsigned col) const override;
      auto SetValueByRow(const wxVariant&, unsigned, unsigned) -> bool override;
      auto GetCount() const -> unsigned int override;
   };

   using DataViewModelPtr = CtDataViewModel::ModelPtr;

} // namespace ctb::app