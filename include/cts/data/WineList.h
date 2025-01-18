#pragma once

#include "cts/constants.h"
#include "cts/Error.h"

#pragma warning(push)
#pragma warning(disable: 4365 4464 4702)
#include <cts/external/csv.hpp>
#pragma warning(pop)

#include <magic_enum/magic_enum.hpp>
#include <cstdint>
#include <deque>
#include <expected>
#include <string>
#include <variant>


namespace cts::data::detail
{
   /// @brief  this struct contains the data values read from the CSV file.
   struct WineListRec
   {
      uint64_t iWineID{};
      std::string WineName{};
      std::string Locale{};
      uint16_t Vintage{};
      uint16_t Quantity{};
      uint16_t Pending{};
      std::string Size{};
      double Price{};
      double Valuation{};
      std::string Country{};
      std::string Region{};
      std::string SubRegion{};
      std::string Appellation{};
      std::string Producer{};
      std::string SortProducer{};
      std::string Color{};
      std::string Category{};
      std::string MasterVarietal{};
      double CTScore{};
      double MYScore{};
      uint16_t BeginConsume{};
      uint16_t EndConsume{};
   };

} // namespace cts::data::detail 


namespace cts::data
{
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
      /// on the record class layout.
     enum class WineListProp : uint32_t
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
      };

      /// @brief static function to get the 0-based index of the last column.
      static constexpr int maxPropIndex() { return static_cast<int>(WineListProp::EndConsume); }

      /// @brief variant that can hold any of our supported field types.
      using ValueWrapper = std::variant<uint16_t, uint64_t, double, std::string>;

      /// @brief used to return a field value or an error
      using ValueResult = std::expected<ValueWrapper, Error>;

      /// @brief get the property corresponding to the specified enum identifier
      ValueResult getProperty(WineListProp prop) const;

      /// @brief array syntax for getting a property value
      ValueResult operator[](WineListProp prop) const 
      {
         return getProperty(prop);
      }

      /// @brief array syntax for getting a
      ///
      /// if the specified index doesn't match an enum identifier, an error will be returned.
      ValueResult operator[](int idx) const 
      {
         auto e = magic_enum::enum_cast<WineListProp>(static_cast<size_t>(idx));
         if (e)
            return getProperty(e.value());
         else
            return std::unexpected{ Error{constants::ERROR_INVALID_PROP_INDEX} };
      }

      uint64_t wineID() const             { return m_rec.iWineID;          }
      std::string wineName() const        { return m_rec.WineName;         }
      std::string locale() const          { return m_rec.Locale;           }
      uint16_t vintage() const            { return m_rec.Vintage;          }
      uint16_t qtyAvailable() const       { return m_rec.Quantity;         }
      uint16_t qtyPending() const         { return m_rec.Pending;          }
      std::string size() const            { return m_rec.Size;             }
      double price() const                { return m_rec.Price;            }
      double valuation() const            { return m_rec.Valuation;        }
      std::string country() const         { return m_rec.Country;          }
      std::string region() const          { return m_rec.Region;           }
      std::string subRegion() const       { return m_rec.SubRegion;        }
      std::string appellation() const     { return m_rec.Appellation;      }
      std::string producer() const        { return m_rec.Producer;         }
      std::string sortProducer() const    { return m_rec.SortProducer;     }
      std::string color() const           { return m_rec.Color;            }
      std::string category() const        { return m_rec.Category;         }
      std::string masterVarietal() const  { return m_rec.MasterVarietal;   }
      double ctScore() const              { return m_rec.CTScore;          }
      double myScore() const              { return m_rec.MYScore;          }
      uint16_t beginConsume() const       { return m_rec.BeginConsume;     }
      uint16_t endConsume() const         { return m_rec.EndConsume;       }

      bool parse(const csv::CSVRow& row);

   private:
      detail::WineListRec m_rec{};
   };

   using WineListData = std::deque<WineListEntry>;
}
