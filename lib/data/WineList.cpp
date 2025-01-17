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
         m_rec.iWineID           = row[to_underlying(WineListProp::iWineID)].get<decltype(m_rec.iWineID)>();
         m_rec.WineName          = row[to_underlying(WineListProp::WineName)].get<decltype(m_rec.WineName)>();
         m_rec.Locale            = row[to_underlying(WineListProp::Locale)].get<decltype(m_rec.Locale)>();
         m_rec.Vintage           = row[to_underlying(WineListProp::Vintage)].get<decltype(m_rec.Vintage)>();
         m_rec.Quantity          = row[to_underlying(WineListProp::Quantity)].get<decltype(m_rec.Quantity)>();
         m_rec.Pending           = row[to_underlying(WineListProp::Pending)].get<decltype(m_rec.Pending)>();
         m_rec.Size              = row[to_underlying(WineListProp::Size)].get<decltype(m_rec.Size)>();
         m_rec.Price             = row[to_underlying(WineListProp::Price)].get<decltype(m_rec.Price)>();
         m_rec.Valuation         = row[to_underlying(WineListProp::Valuation)].get<decltype(m_rec.Valuation)>();
         m_rec.Country           = row[to_underlying(WineListProp::Country)].get<decltype(m_rec.Country)>();
         m_rec.Region            = row[to_underlying(WineListProp::Region)].get<decltype(m_rec.Region)>();
         m_rec.SubRegion         = row[to_underlying(WineListProp::SubRegion)].get<decltype(m_rec.SubRegion)>();
         m_rec.Appellation       = row[to_underlying(WineListProp::Appellation)].get<decltype(m_rec.Appellation)>();
         m_rec.Producer          = row[to_underlying(WineListProp::Producer)].get<decltype(m_rec.Producer)>();
         m_rec.SortProducer      = row[to_underlying(WineListProp::SortProducer)].get<decltype(m_rec.SortProducer)>();
         m_rec.Color             = row[to_underlying(WineListProp::Color)].get<decltype(m_rec.Color)>();
         m_rec.Category          = row[to_underlying(WineListProp::Category)].get<decltype(m_rec.Category)>();
         m_rec.MasterVarietal    = row[to_underlying(WineListProp::MasterVarietal)].get<decltype(m_rec.MasterVarietal)>();
         m_rec.BeginConsume      = row[to_underlying(WineListProp::BeginConsume)].get<decltype(m_rec.BeginConsume)>();
         m_rec.EndConsume        = row[to_underlying(WineListProp::EndConsume)].get<decltype(m_rec.EndConsume)>();

         long double val{};
         if (row[to_underlying(WineListProp::CTScore)].try_parse_decimal(val))
            m_rec.CTScore = static_cast<double>(val);
         else
            m_rec.CTScore = 0;

         if (row[to_underlying(WineListProp::MYScore)].try_parse_decimal(val))
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

} // namespace cts::data
