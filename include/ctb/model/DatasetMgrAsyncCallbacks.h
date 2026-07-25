#pragma once
#include "ctb/ctb.h"
#include "ctb/tables/table_data.h"
#include <expected>
#include <functional>


namespace ctb
{

   /// @brief notification message that gets sent via callback after a table is downloaded
   struct TableDownloadInfo
   {
      TableId table_id{};
      size_t  file_size{};
   };

   // Used for downloading table files, expected value is the contents of the file
   using TableResult         = std::expected<TableDownloadInfo, ctb::Error>;
   using TableResultCallback = copyable_function<void(TableResult) const>;


   /// @brief Notification message that ges  sent via callback after a label image is downloaded
   ///        Contains the iamge bytes as well as file metadata
   struct ImageFileContents
   {
      uint64_t    wine_id{};     // wine_id the image is for
      Buffer      data{};        // binary contents of the file
      fs::path    file_path{};   // path the file was loaded from, or path it should be saved to if image was downloaded
      MaybeString url{};         // will only have a value if this file was downlaoded and should be saved. If empty file was read from disk
   };

   // used for loading (or downloading) images, expected value is the image bytes.
   using ImageResult         = std::expected<ImageFileContents, ctb::Error>;
   using ImageResultCallback = copyable_function<void(ImageResult) const>;



}   // namespace ctb
