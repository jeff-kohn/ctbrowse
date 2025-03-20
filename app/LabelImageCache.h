#pragma once

#include "App.h"

#include <wx/bitmap.h>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>


namespace ctb
{
   namespace fs = std::filesystem;

   /// @brief manages a disk-based cache of wine label images.
   ///
   class LabelImageCache final
   {
   public:
      explicit LabelImageCache(fs::path cache_folder);
      ~LabelImageCache() noexcept;


      /// @brief get a label image from the cache, if available
      /// @return the image if available, or std::nullopt if not
      std::optional<wxBitmap> getLabelImage(uint64_t wine_id);


      /// @brief fetches label images missing from the cache for the specified iWineId's
      /// 
      /// this is an asynchronous background 
      void requestCacheUpdate(std::vector<uint64_t> wine_ids);


      /// @brief check if a background cache update is currently running
      /// 
      bool pendingCacheUpdate() const
      {
         return false;
      }


      /// @brief cancel any pending cache downloads
      ///
      void cancelCacheUpdate() {}


      // no default init or copy/assign
      LabelImageCache() = delete;
      LabelImageCache(const LabelImageCache&) = delete;
      LabelImageCache(LabelImageCache&&) = default;
      LabelImageCache& operator=(const LabelImageCache&) = delete;
      LabelImageCache& operator=(LabelImageCache&&) = default;

   private:
      fs::path m_cache_folder{};


      static std::string buildFileName(uint64_t wine_id, int image_num = 1)
      {
         return std::format(constants::FMT_LABEL_IMAGE_FILENAME, wine_id, image_num);
      }

      static fs::path buildFilePath(fs::path folder, uint64_t wine_id, int image_num = 1)
      {
         return folder / buildFileName(wine_id, image_num);
      }
   };
}