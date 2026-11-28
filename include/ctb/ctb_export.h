#include "ctb/ctb.h"
#include "ctb/interfaces/IDataset.h"
#include "ctb/tables/CtSchema.h"
#include <sstream>

namespace ctb
{

   struct ExportFieldMapping
   {};

   /// @brief serialize a dataset to columnar text format.
   /// @param ds       - dataset to export
   /// @param props    - properties to include
   /// @param sep_char - separator character to use.
   /// @return the requested text.
   /// @throw ctb::Error
   [[nodiscard]] std::string serializeDataset(const DatasetPtr& ds, std::span<const CtProp> props, char sep_char = '\t') noexcept(false);


   /// @brief Export a dataset to columnar text format, with specified column names
   /// @param ds       - dataset to export
   /// @param props    - properties to include and the names to use for their column headings
   /// @param sep_char - separator character to use.
   /// @return the requested text.
   /// @throw ctb::Error
   [[nodiscard]] std::string serializeDataset(const DatasetPtr&                         ds,
                                              std::span<std::pair<CtProp, std::string>> prop_mappings,
                                              char                                      sep_char = '\t') noexcept(false);

   void exportDatasetToFile(const fs::path& target_path, const DatasetPtr& ds, std::span<const CtProp> props, char sep_char = '\t');
}   // namespace ctb

