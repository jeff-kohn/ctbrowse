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

      /// @brief Create a new model for the supplied DatasetPtr
      /// @return smart ptr to the newly created model object
      [[nodiscard]] static auto create(DatasetPtr dataset = {}) -> ModelPtr;

      /// @brief Returns the active dataset for this model (may be empty/null)
      auto getDataset() -> DatasetPtr;

      /// @brief Associates a new dataset with this model
      void setDataset(DatasetPtr dataset);

      /// @brief Forces a refresh of the data view after large changes to underlying dataset
      void reQuery();

      /// @brief associate a data view ctrl with this model. we only support one associated view, last call wins.
      void associateView(wxDataViewCtrl* view);

      // we only expose a limited amount of base-class API
      using base::GetItem;
      using base::GetRow;

   private:
      DatasetPtr       m_dataset{};

      explicit CtDataViewModel(DatasetPtr dataset = {}) : m_dataset{ dataset }
      {}

      // instrusive ref counting in wxWidgets means smart ptr need access to ref-counting methods.
      friend ModelPtr;

      // these are the real purpose of this class, they're called by the base class to
      // provide data when the list-view needs it.
      void GetValueByRow(wxVariant& variant, unsigned row, unsigned col) const override;
      auto SetValueByRow(const wxVariant&, unsigned, unsigned) -> bool override;
      auto GetCount() const -> unsigned int override;
   };

   using DataViewModelPtr = CtDataViewModel::ModelPtr;

} // namespace ctb::app