#include "LabelImageCache.h"
#include <stdexec/execution.hpp>

namespace ctb
{


   LabelImageCache::LabelImageCache(fs::path cache_folder) : m_cache_folder(std::move(cache_folder))
   {
      if (m_cache_folder.is_absolute())
         throw Error{ constants::ERROR_STR_RELATIVE_LABEL_CACHE };

      if (!fs::exists(cache_folder) or !fs::is_directory(cache_folder))
      {
         if (!fs::create_directories(cache_folder))
            throw Error{ constants::FMT_ERROR_NO_LABEL_CACHE_FOLDER };
      }
   }


   LabelImageCache::~LabelImageCache() noexcept
   {
      try 
      {
         // TODO: Revisit whether this can actually throw once implemented.
         cancelCacheUpdate();
      }
      catch(...){}
   }


   std::optional<wxBitmap> LabelImageCache::getLabelImage(uint64_t wine_id)
   {
      // if we decide to support getting multiple images per wine, this could become a parameter.
      constexpr auto img_num = 1;

      auto file_path = buildFilePath(m_cache_folder, wine_id, img_num);
      
      wxBitmap img{};
      if (fs::exists(file_path) and img.LoadFile(file_path.generic_string().c_str(), wxBITMAP_TYPE_JPEG))
         return img;

      return {};
   }


   void LabelImageCache::requestCacheUpdate(std::vector<uint64_t> wine_ids)
   {
      // launch background thread to do the work and return immediately.
   }


} // namespace ctb
