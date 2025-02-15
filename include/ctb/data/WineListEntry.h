/*******************************************************************
 * @file WineListEntry.h
 *
 * @brief Header file for the WineListEntry class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/functors.h"
#include "ctb/data/table_data.h"
#include "ctb/data/TableProperty.h"

#include <magic_enum/magic_enum.hpp>
#include <cstdint>
#include <deque>
#include <expected>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <variant>


namespace ctb::data
{


   namespace detail
   {
      /// @brief  this struct contains the data values read from the CSV file.
      struct WineListRec
      {
         uint64_t iWineID{};
         std::string WineName{};
         std::string Locale{};
         uint16_t Vintage{};
         NullableShort Quantity{};
         NullableShort Pending{};
         std::string Size{};
         NullableDouble Price{};
         NullableDouble Valuation{};
         std::string Country{};
         std::string Region{};
         std::string SubRegion{};
         std::string Appellation{};
         std::string Producer{};
         std::string SortProducer{};
         std::string Color{};
         std::string Category{};
         std::string MasterVarietal{};
         NullableDouble CTScore{};
         NullableDouble MYScore{};
         NullableShort BeginConsume{};
         NullableShort EndConsume{};
      };

   } // namespace detail 


   /// @brief class that encapsulates the data from a row in a CellarTracker 'List' CSV file.
   ///
   /// currently the class only support parsing from CSV. Other formats could be supported
   /// in the future with additional parse() methods.
   /// 
   /// all string properties are returned as string_view for performance, which means such
   /// objects are only valid for the lifetime of this object.
   /// 
   class WineListEntry
   {
   public:
      
      /// @brief construct a WineListEntry from a row in a CSV data file
      /// 
      explicit WineListEntry(const csv::CSVRow& row);


      /// @brief these are the fields from the denormalized CSV that we parse and use
      ///
      /// the values map to column indices in the file, while the enum ordering is based
      /// on the record class layout. The enums with values starting at 100 are calculated
      /// fields, and do not map directly to a column in the file.
      ///
      enum class Prop : uint32_t
      {
         iWineID = 0,
         WineName = 13,
         Locale = 14,
         Vintage = 12,
         Quantity = 2,
         Pending = 3,
         Size = 4,
         Price = 5,
         Valuation = 6,
         Country = 15,
         Region = 16,
         SubRegion = 17,
         Appellation = 18,
         Producer = 19,
         SortProducer = 20,
         Color = 22,
         Category = 23,
         MasterVarietal = 25,
         CTScore = 59,
         MYScore = 61,
         BeginConsume = 63,
         EndConsume = 64,
         WineAndVintage = 100
      };

      
      /// @brief small helper to convert a Prop enum into its integer index
      /// 
      static constexpr int propToIndex(Prop prop)
      {
         return enumToIndex(prop);
      }


      /// @brief small helper to convert a zero-based index to a Prop enum
      /// 
      static constexpr Prop propFromIndex(int idx)
      {
         return enumFromIndex<Prop>(idx);
      }

      // type alias used by template code
      ///
      using RowType = csv::CSVRow;


      /// @brief static function to get the 0-based index of the last column.
      ///
      static constexpr int maxPropIndex() { return static_cast<int>(Prop::WineAndVintage); }


      /// @brief used to return a field value or an error
      ///
      using PropertyResult = std::expected<TableProperty, Error>;


      /// @brief get the property corresponding to the specified enum identifier
      ///
      [[nodiscard]] PropertyResult getProperty(Prop prop) const;


      /// @brief array syntax for getting a property value
      ///
      [[nodiscard]] PropertyResult operator[](Prop prop) const 
      {
         return getProperty(prop);
      }


      /// string-based properties return a string_view, which will only be valid for the lifetime of this object.
      /// 
      /// 
      uint64_t wineID() const                  { return m_rec.iWineID;          }
      std::string_view wineName() const        { return m_rec.WineName;         }
      std::string_view locale() const          { return m_rec.Locale;           }
      uint16_t vintage() const                 { return m_rec.Vintage;          }
      NullableShort qtyAvailable() const       { return m_rec.Quantity;         }
      NullableShort qtyPending() const         { return m_rec.Pending;          }
      std::string_view size() const            { return m_rec.Size;             }
      NullableDouble price() const             { return m_rec.Price;            }
      NullableDouble valuation() const         { return m_rec.Valuation;        }
      std::string_view country() const         { return m_rec.Country;          }
      std::string_view region() const          { return m_rec.Region;           }
      std::string_view subRegion() const       { return m_rec.SubRegion;        }
      std::string_view appellation() const     { return m_rec.Appellation;      }
      std::string_view producer() const        { return m_rec.Producer;         }
      std::string_view sortProducer() const    { return m_rec.SortProducer;     }
      std::string_view color() const           { return m_rec.Color;            }
      std::string_view category() const        { return m_rec.Category;         }
      std::string_view masterVarietal() const  { return m_rec.MasterVarietal;   }
      NullableDouble ctScore() const           { return m_rec.CTScore;          }
      NullableDouble myScore() const           { return m_rec.MYScore;          }
      NullableShort beginConsume() const       { return m_rec.BeginConsume;     }
      NullableShort endConsume() const         { return m_rec.EndConsume;       }
      std::string_view wineAndVintage() const  { return m_wine_and_vintage;     }


      /// @brief parses data from a row in the CSV file to this object.
      /// 
      /// returns false if the row could not be parse, in which case this 
      /// object may be in an invalid/indeterminate state (but not UB)
      ///
      bool parse(const csv::CSVRow& row);


      WineListEntry() = default;
      WineListEntry(const WineListEntry&) = default;
      WineListEntry(WineListEntry&&) = default;
      WineListEntry& operator=(const WineListEntry&) = default;
      WineListEntry& operator=(WineListEntry&&) = default;

   private:
      detail::WineListRec m_rec{};
      std::string m_wine_and_vintage{}; // calclulated value that we cache.
   };

   using WineListData = std::deque<WineListEntry>;
}
