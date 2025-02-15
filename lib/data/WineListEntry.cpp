/*********************************************************************
* @file       WineListEntry.cpp
*
* @brief      implementation for the class WineListEntry
*
* @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
*********************************************************************/
#include "ctb/data/WineListEntry.h"
#include <magic_enum/magic_enum.hpp>


namespace ctb::data
{

   /// @brief parses a CSVField into a nullable/optional double
   ///
   /// converts the CSV value to decimal and returns it, or returns 
   /// nullopt if the field was empty or couldn't be parsed
   /// 
   NullableDouble parseDouble(csv::CSVField&& fld)
   {
      long double val{};
      if (fld.get_sv().length() && fld.try_parse_decimal(val))
      {
         return static_cast<double>(val);
      }
      return std::nullopt;
   }


   /// @brief parse a field that may contain a null value
   ///
   /// if value_as_null.has_value(), then a parsed value matching 
   /// value_as_null.value() will be treated as null and std::nullopt will be returned
   /// instead of the parsed value. Allows treating special values as null
   /// 
   template<typename T>
   std::optional<T> parseOptional(csv::CSVField&& fld, std::optional<T> value_as_null = std::nullopt)
   {
      std::optional<T> retval{};

      if (fld.get_sv().length())
      {
         auto parsed_val = fld.get<T>();
         if (value_as_null)
         {
            if (parsed_val != *value_as_null)
            {

               retval = parsed_val;
            }
         }
         else 
         {
            retval = parsed_val;
         }
      }
      return retval;
   }


   WineListEntry::WineListEntry(const csv::CSVRow& row) 
   {
      parse(row);
   }


   bool WineListEntry::parse(const csv::CSVRow& row)
   {
      using namespace detail;
      using std::to_underlying;

      // really hate that such a low-level library uses exceptions, could be very expensive if there
      // are a lot of invalid rows.
      try
      {
         m_rec.iWineID           = row[to_underlying(Prop::iWineID)].get<decltype(m_rec.iWineID)>();
         m_rec.WineName          = row[to_underlying(Prop::WineName)].get<decltype(m_rec.WineName)>();
         m_rec.Locale            = row[to_underlying(Prop::Locale)].get<decltype(m_rec.Locale)>();
         m_rec.Vintage           = row[to_underlying(Prop::Vintage)].get<decltype(m_rec.Vintage)>();
         m_rec.Size              = row[to_underlying(Prop::Size)].get<decltype(m_rec.Size)>();
         m_rec.Country           = row[to_underlying(Prop::Country)].get<decltype(m_rec.Country)>();
         m_rec.Region            = row[to_underlying(Prop::Region)].get<decltype(m_rec.Region)>();
         m_rec.SubRegion         = row[to_underlying(Prop::SubRegion)].get<decltype(m_rec.SubRegion)>();
         m_rec.Appellation       = row[to_underlying(Prop::Appellation)].get<decltype(m_rec.Appellation)>();
         m_rec.Producer          = row[to_underlying(Prop::Producer)].get<decltype(m_rec.Producer)>();
         m_rec.SortProducer      = row[to_underlying(Prop::SortProducer)].get<decltype(m_rec.SortProducer)>();
         m_rec.Color             = row[to_underlying(Prop::Color)].get<decltype(m_rec.Color)>();
         m_rec.Category          = row[to_underlying(Prop::Category)].get<decltype(m_rec.Category)>();
         m_rec.MasterVarietal    = row[to_underlying(Prop::MasterVarietal)].get<decltype(m_rec.MasterVarietal)>();

         m_rec.Quantity          = parseOptional<uint16_t>(row[to_underlying(Prop::Quantity)], 0);
         m_rec.Pending           = parseOptional<uint16_t>(row[to_underlying(Prop::Pending)], 0);
         m_rec.BeginConsume      = parseOptional<uint16_t>(row[to_underlying(Prop::BeginConsume)], 9999);
         m_rec.EndConsume        = parseOptional<uint16_t>(row[to_underlying(Prop::EndConsume)], 9999);

         m_rec.Price             = parseOptional<double>(row[to_underlying(Prop::Price)]);
         m_rec.Valuation         = parseOptional<double>(row[to_underlying(Prop::Valuation)]);
         m_rec.CTScore           = parseOptional<double>(row[to_underlying(Prop::CTScore)]);
         m_rec.MYScore           = parseOptional<double>(row[to_underlying(Prop::MYScore)]);

         // calculated field but we store it's value for perf reason.
         m_wine_and_vintage      = std::format("{} {}", vintage(), wineName());
         return true;
      }
      catch ([[maybe_unused]] std::exception& e)
      {
         return false;
      }
   }

   WineListEntry::PropertyResult WineListEntry::getProperty(Prop prop) const
   {
      using enum Prop;

      switch (prop)
      {
         case iWineID:        return TableProperty{ wineID() };
         case WineName:       return TableProperty{ wineName() };
         case Locale:         return TableProperty{ locale() };
         case Vintage:        return TableProperty{ vintage() };
         case Quantity:       return TableProperty{ qtyAvailable() };
         case Pending:        return TableProperty{ qtyPending() };
         case Size:           return TableProperty{ size() };
         case Price:          return TableProperty{ price() };
         case Valuation:      return TableProperty{ valuation() };
         case Country:        return TableProperty{ country() };
         case Region:         return TableProperty{ region() };
         case SubRegion:      return TableProperty{ subRegion() };
         case Appellation:    return TableProperty{ appellation() };
         case Producer:       return TableProperty{ producer() };
         case SortProducer:   return TableProperty{ sortProducer() };
         case Color:          return TableProperty{ color() };
         case Category:       return TableProperty{ category() };
         case MasterVarietal: return TableProperty{ masterVarietal() };
         case CTScore:        return TableProperty{ ctScore() };
         case MYScore:        return TableProperty{ myScore() };
         case BeginConsume:   return TableProperty{ beginConsume() };
         case EndConsume:     return TableProperty{ endConsume() };
         case WineAndVintage: return TableProperty{ wineAndVintage() };
         default:
            return std::unexpected{ Error{constants::ERROR_STR_INVALID_INDEX} };
      }
   }

} // namespace ctb::data
