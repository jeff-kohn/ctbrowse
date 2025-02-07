/*******************************************************************
 * @file WineListEntry.h
 *
 * @brief Header file for the WineListEntry class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once

#include "ctb/ctb.h"
#include "ctb/data/table_data.h"

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
      WineListEntry() = default;
      WineListEntry(const csv::CSVRow& row);
      WineListEntry(const WineListEntry&) = default;
      WineListEntry(WineListEntry&&) = default;

      WineListEntry& operator=(const WineListEntry&) = default;
      WineListEntry& operator=(WineListEntry&&) = default;

      /// @brief these are the fields from the denormalized CSV that we parse and use
      ///
      /// the values map to column indices in the file, while the enum ordering is based
      /// on the record class layout. The enums with values starting at 100 are calculated
      /// fields, and do not map directly to a column in the file.
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

      
      static constexpr int to_int(Prop prop)
      {
         return static_cast<int>(std::to_underlying(prop));
      }

      // type alias used by template code
      using RowType = csv::CSVRow;


      /// @brief static function to get the 0-based index of the last column.
      static constexpr int maxPropIndex() { return static_cast<int>(Prop::WineAndVintage); }


      /// @brief variant that can hold any of our supported field types.
      using ValueWrapper = std::variant<uint16_t, uint64_t, NullableDouble, NullableShort, std::string_view>;


      /// @brief used to return a field value or an error
      using ValueResult = std::expected<ValueWrapper, Error>;


      /// @brief get the property corresponding to the specified enum identifier
      [[nodiscard]] ValueResult getProperty(Prop prop) const;


      /// @brief array syntax for getting a property value
      [[nodiscard]] ValueResult operator[](Prop prop) const 
      {
         return getProperty(prop);
      }


      /// @brief array syntax for getting a property value
      ///
      /// if the specified index doesn't match an enum identifier, an error will be returned.
      [[nodiscard]] ValueResult operator[](std::integral auto idx) const 
      {
         auto e = magic_enum::enum_value<Prop>(static_cast<int>(idx));
         if (e)
            return getProperty(e.value());
         else
            return std::unexpected{ Error{constants::ERROR_STR_INVALID_INDEX} };
      }


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
      

      /// @brief calculated field to show vintage and wine name as a single value
      std::string_view wineAndVintage() const  { return m_wine_and_vintage;     }


      /// @brief parses data from a row in the CSV file to this object.
      /// 
      /// returns false if the row could not be parse, in which case this 
      /// object may be in an invalid/indeterminate state (but not UB)
      bool parse(const csv::CSVRow& row);

   private:
      detail::WineListRec m_rec{};
      std::string m_wine_and_vintage{}; //
   };

   using WineListData = std::deque<WineListEntry>;
}
