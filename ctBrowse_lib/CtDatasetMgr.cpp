#include "ctb/model/CtDatasetMgr.h"

#include "async/DatasetMgrAsyncImpl.h"
#include "ctb/model/CtDataset.h"
#include "ctb/model/ProReviewsCache.h"
#include "ctb/tables/BottleInventoryTraits.h"
#include "ctb/tables/ConsumedWineTraits.h"
#include "ctb/tables/PendingWineTraits.h"
#include "ctb/tables/PrivateNotesTraits.h"
#include "ctb/tables/PurchasedWineTraits.h"
#include "ctb/tables/ReadyToDrinkTraits.h"
#include "ctb/tables/TaggedWinesTraits.h"
#include "ctb/tables/TastingNotesTraits.h"
#include "ctb/tables/WineListTraits.h"
#include "ctb/tables/table_data.h"

#include <asio/awaitable.hpp>
#include <asio/read.hpp>
#include <asio/use_awaitable.hpp>
#include <exec/static_thread_pool.hpp>
#include <fmt/std.h>

#include <exception>


namespace ctb
{
   using namespace ctb::senders;

   namespace
   {

#ifndef ERROR_FILE_NOT_FOUND
      constexpr long ERROR_FILE_NOT_FOUND = 2L;
#endif   // !ERROR_FILE_NOT_FOUND


      /// @brief Attempts to load a dataset from file path, throws on error.
      template<typename TableT>
      auto getDatasetOrThrow(const fs::path& folder, TableId tbl_id) -> DatasetPtr
      {
         auto result = loadTableData<TableT>(folder, tbl_id);
         if (!result) throw Error{ result.error() };

         return CtDataset<TableT>::create(move(result.value()));
      }

   }   // namespace


   CtDatasetMgr::CtDatasetMgr() = default;


   CtDatasetMgr::~CtDatasetMgr() noexcept
   {
      try
      {
         m_impl->requestShutdown();
      }
      catch (...) // NOLINT
      {}   
   }


   CtDatasetMgr::CtDatasetMgr(const DatasetMgrOptions& opts) noexcept(false)
   {
      init(opts);
   }


   void CtDatasetMgr::init(const DatasetMgrOptions& opts)
   {
      if (!opts.browser_path.empty()) m_impl->startBrowser(opts.browser_path, opts.browser_data_dir, opts.browser_ws_port);
      if (!opts.table_folder.empty()) setTableFolder(opts.table_folder);
      if (!opts.label_folder.empty()) setLabelImageFolder(opts.label_folder);
   }


   auto CtDatasetMgr::setTableFolder(const std::string& folder) noexcept(false) -> CtDatasetMgr&
   {
      fs::path folder_path{ expandEnvironmentVars(folder) };
      return setTableFolder(fs::path{ folder_path });
   }


   auto CtDatasetMgr::setTableFolder(const fs::path& folder) noexcept(false) -> CtDatasetMgr&
   {
      if (!fs::exists(folder) and !createFolderPath(folder))
      {
         throw Error{ ERROR_PATH_NOT_FOUND, Error::Category::DatasetError, constants::FMT_ERROR_PATH_NOT_FOUND, folder.generic_string() };
      }
      m_impl->setTableFolder(folder.generic_string());
      return *this;
   }


   auto CtDatasetMgr::getTableFolder() const -> const std::string&
   {
      return m_impl->getTableFolder();
   }


   auto CtDatasetMgr::setLabelImageFolder(const std::string& folder) noexcept(false) -> CtDatasetMgr&
   {
      return setLabelImageFolder(fs::path{ expandEnvironmentVars(folder) });
   }


   auto CtDatasetMgr::setLabelImageFolder(const fs::path& folder) noexcept(false) -> CtDatasetMgr&
   {
      if (!fs::exists(folder) and !createFolderPath(folder))
      {
         throw Error{ ERROR_PATH_NOT_FOUND, Error::Category::DatasetError, constants::FMT_ERROR_NO_LABEL_CACHE_FOLDER,
                      folder.generic_string() };
      }
      m_impl->setLabelImageFolder(folder.generic_string());
      return *this;
   }


   auto CtDatasetMgr::getLabelImageFolder() const -> const std::string&
   {
      return m_impl->getLabelImageFolder();
   }

   bool CtDatasetMgr::requestShutdown()
   {
      return m_impl->requestShutdown();
   }


   bool CtDatasetMgr::shutdownRequested() const
   {
      return m_impl->shutdownRequested();
   }


   auto CtDatasetMgr::loadDataset(TableId table_id) -> DatasetPtr
   {
      switch (table_id)
      {
         case TableId::List        : return getDatasetOrThrow<WineListTable>(getTableFolder(), table_id);
         case TableId::Pending     : return getDatasetOrThrow<PendingWineTable>(getTableFolder(), table_id);
         case TableId::Consumed    : return getDatasetOrThrow<ConsumedWineTable>(getTableFolder(), table_id);
         case TableId::Availability: return getDatasetOrThrow<ReadyToDrinkTable>(getTableFolder(), table_id);
         case TableId::Purchase    : return getDatasetOrThrow<PurchasedWineTable>(getTableFolder(), table_id);
         case TableId::Tag         : return getDatasetOrThrow<TaggedWinesTable>(getTableFolder(), table_id);
         case TableId::Inventory   : return getDatasetOrThrow<BottleInventoryTable>(getTableFolder(), table_id);
         case TableId::PrivateNotes: return getDatasetOrThrow<PrivateNotesTable>(getTableFolder(), table_id);
         case TableId::Notes       : return getDatasetOrThrow<TastingNotesTable>(getTableFolder(), table_id);
         default                   : throw Error{ "Table not found." };
      };
   }


   /// @brief Load a dataset and apply options
   auto CtDatasetMgr::loadDataset(const CtDatasetOptions& options) -> DatasetPtr
   {
      // load dataset and then apply options.
      auto dataset = loadDataset(options.table_id);
      options.applyToDataset(dataset);
      return dataset;
   }


   auto CtDatasetMgr::getProReviewsCache() -> ProReviewsCache&
   {
      if (!m_pro_cache)
      {
         m_pro_cache = loadTableData<ProReviewsCacheTable>(getTableFolder(), TableId::Availability).value_or({});
      }
      return m_pro_cache.value();
   }


   void CtDatasetMgr::downloadTableAsync(TableId table_id, const CredentialWrapper& cred, TableResultCallback notify_callback)
   {
      if (!shutdownRequested()) m_impl->downloadTableAsync(table_id, cred, move(notify_callback));
   }


   void CtDatasetMgr::retrieveLabelImageAsync(uint64_t wine_id, ImageResultCallback result_callback)
   {
      if (!shutdownRequested()) m_impl->retrieveLabelImageAsync(wine_id, move(result_callback));
   }


   void CtDatasetMgr::checkBrowserLoginAsync(CredentialWrapper cred, LoginResultCallback result_callback)
   {
      if (!shutdownRequested()) m_impl->checkBrowserLoginAsync(move(cred), move(result_callback));
   }


}   // namespace ctb

