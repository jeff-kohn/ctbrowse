#include "cts/data/WineList.h"
#include <magic_enum/magic_enum.hpp>


namespace cts::data
{

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
         m_rec.iWineID           = row[to_underlying(Props::iWineID)].get<decltype(m_rec.iWineID)>();
         m_rec.WineName          = row[to_underlying(Props::WineName)].get<decltype(m_rec.WineName)>();
         m_rec.Locale            = row[to_underlying(Props::Locale)].get<decltype(m_rec.Locale)>();
         m_rec.Vintage           = row[to_underlying(Props::Vintage)].get<decltype(m_rec.Vintage)>();
         m_rec.Quantity          = row[to_underlying(Props::Quantity)].get<decltype(m_rec.Quantity)>();
         m_rec.Pending           = row[to_underlying(Props::Pending)].get<decltype(m_rec.Pending)>();
         m_rec.Size              = row[to_underlying(Props::Size)].get<decltype(m_rec.Size)>();
         m_rec.Price             = row[to_underlying(Props::Price)].get<decltype(m_rec.Price)>();
         m_rec.Valuation         = row[to_underlying(Props::Valuation)].get<decltype(m_rec.Valuation)>();
         m_rec.Country           = row[to_underlying(Props::Country)].get<decltype(m_rec.Country)>();
         m_rec.Region            = row[to_underlying(Props::Region)].get<decltype(m_rec.Region)>();
         m_rec.SubRegion         = row[to_underlying(Props::SubRegion)].get<decltype(m_rec.SubRegion)>();
         m_rec.Appellation       = row[to_underlying(Props::Appellation)].get<decltype(m_rec.Appellation)>();
         m_rec.Producer          = row[to_underlying(Props::Producer)].get<decltype(m_rec.Producer)>();
         m_rec.SortProducer      = row[to_underlying(Props::SortProducer)].get<decltype(m_rec.SortProducer)>();
         m_rec.Color             = row[to_underlying(Props::Color)].get<decltype(m_rec.Color)>();
         m_rec.Category          = row[to_underlying(Props::Category)].get<decltype(m_rec.Category)>();
         m_rec.MasterVarietal    = row[to_underlying(Props::MasterVarietal)].get<decltype(m_rec.MasterVarietal)>();
         m_rec.BeginConsume      = row[to_underlying(Props::BeginConsume)].get<decltype(m_rec.BeginConsume)>();
         m_rec.EndConsume        = row[to_underlying(Props::EndConsume)].get<decltype(m_rec.EndConsume)>();

         long double val{};
         if (row[to_underlying(Props::CTScore)].try_parse_decimal(val))
            m_rec.CTScore = static_cast<double>(val);
         else
            m_rec.CTScore = 0;

         if (row[to_underlying(Props::MYScore)].try_parse_decimal(val))
            m_rec.MYScore = static_cast<double>(val);
         else
            m_rec.MYScore = 0;

         return true;
      }
      catch (std::exception&)
      {
         return false;
      }
   }

   WineListEntry::ValueResult WineListEntry::getProperty(Props prop) const
   {
      using enum Props;

      switch (prop)
      {
         case iWineID:        return m_rec.iWineID;
         case WineName:       return m_rec.WineName;
         case Locale:         return m_rec.Locale;
         case Vintage:        return m_rec.Vintage;
         case Quantity:       return m_rec.Quantity;
         case Pending:        return m_rec.Pending;
         case Size:           return m_rec.Size;
         case Price:          return m_rec.Price;
         case Valuation:      return m_rec.Valuation;
         case Country:        return m_rec.Country;
         case Region:         return m_rec.Region;
         case SubRegion:      return m_rec.SubRegion;
         case Appellation:    return m_rec.Appellation;
         case Producer:       return m_rec.Producer;
         case SortProducer:   return m_rec.SortProducer;
         case Color:          return m_rec.Color;
         case Category:       return m_rec.Category;
         case MasterVarietal: return m_rec.MasterVarietal;
         case BeginConsume:   return m_rec.BeginConsume;
         case EndConsume:     return m_rec.EndConsume;
         default:
            return std::unexpected{ Error{constants::ERROR_INVALID_PROP_INDEX} };
      };
   }

} // namespace cts::data
