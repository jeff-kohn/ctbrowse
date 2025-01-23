#include "ctb/data/WineListEntry.h"
#include <magic_enum/magic_enum.hpp>


namespace ctb::data
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
         m_rec.iWineID           = row[to_underlying(Prop::iWineID)].get<decltype(m_rec.iWineID)>();
         m_rec.WineName          = row[to_underlying(Prop::WineName)].get<decltype(m_rec.WineName)>();
         m_rec.Locale            = row[to_underlying(Prop::Locale)].get<decltype(m_rec.Locale)>();
         m_rec.Vintage           = row[to_underlying(Prop::Vintage)].get<decltype(m_rec.Vintage)>();
         m_rec.Quantity          = row[to_underlying(Prop::Quantity)].get<decltype(m_rec.Quantity)>();
         m_rec.Pending           = row[to_underlying(Prop::Pending)].get<decltype(m_rec.Pending)>();
         m_rec.Size              = row[to_underlying(Prop::Size)].get<decltype(m_rec.Size)>();
         m_rec.Price             = row[to_underlying(Prop::Price)].get<decltype(m_rec.Price)>();
         m_rec.Valuation         = row[to_underlying(Prop::Valuation)].get<decltype(m_rec.Valuation)>();
         m_rec.Country           = row[to_underlying(Prop::Country)].get<decltype(m_rec.Country)>();
         m_rec.Region            = row[to_underlying(Prop::Region)].get<decltype(m_rec.Region)>();
         m_rec.SubRegion         = row[to_underlying(Prop::SubRegion)].get<decltype(m_rec.SubRegion)>();
         m_rec.Appellation       = row[to_underlying(Prop::Appellation)].get<decltype(m_rec.Appellation)>();
         m_rec.Producer          = row[to_underlying(Prop::Producer)].get<decltype(m_rec.Producer)>();
         m_rec.SortProducer      = row[to_underlying(Prop::SortProducer)].get<decltype(m_rec.SortProducer)>();
         m_rec.Color             = row[to_underlying(Prop::Color)].get<decltype(m_rec.Color)>();
         m_rec.Category          = row[to_underlying(Prop::Category)].get<decltype(m_rec.Category)>();
         m_rec.MasterVarietal    = row[to_underlying(Prop::MasterVarietal)].get<decltype(m_rec.MasterVarietal)>();
         m_rec.BeginConsume      = row[to_underlying(Prop::BeginConsume)].get<decltype(m_rec.BeginConsume)>();
         m_rec.EndConsume        = row[to_underlying(Prop::EndConsume)].get<decltype(m_rec.EndConsume)>();

         long double val{};
         if (row[to_underlying(Prop::CTScore)].try_parse_decimal(val))
            m_rec.CTScore = static_cast<double>(val);
         else
            m_rec.CTScore = 0;

         if (row[to_underlying(Prop::MYScore)].try_parse_decimal(val))
            m_rec.MYScore = static_cast<double>(val);
         else
            m_rec.MYScore = 0;

         return true;
      }
      catch ([[maybe_unused]] std::exception& e)
      {
         return false;
      }
   }

   WineListEntry::ValueResult WineListEntry::getProperty(Prop prop) const
   {
      using enum Prop;

      switch (prop)
      {
         case iWineID:        return wineID();
         case WineName:       return wineName();
         case Locale:         return locale();
         case Vintage:        return vintage();
         case Quantity:       return qtyAvailable();
         case Pending:        return qtyPending();
         case Size:           return size();
         case Price:          return price();
         case Valuation:      return valuation();
         case Country:        return country();
         case Region:         return region();
         case SubRegion:      return subRegion();
         case Appellation:    return appellation();
         case Producer:       return producer();
         case SortProducer:   return sortProducer();
         case Color:          return color();
         case Category:       return category();
         case MasterVarietal: return masterVarietal();
         case CTScore:        return ctScore();
         case MYScore:        return myScore();
         case BeginConsume:   return beginConsume();
         case EndConsume:     return endConsume();
         case WineAndVintage: return wineAndVintage();
         default:
            return std::unexpected{ Error{constants::ERROR_INVALID_PROP_INDEX} };
      };
   }

} // namespace ctb::data
