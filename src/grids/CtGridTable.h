#pragma once

#include "cts/Error.h"
#include "cts/functors.h"


#include <wx/grid.h>
#include <expected>
#include <format>
#include <memory>
#include <variant>

namespace cts
{

   inline auto FieldToStr = Overloaded{
      [](uint64_t val) { return wxString{ std::format("{}", val)};   },
      [](uint16_t val) { return wxString{ std::format("{}", val)};   },
      [](double val) { return wxString{ std::format("{}", val)};   },
      [](const std::string& val) { return wxString{ std::format("{}", val)};   }
   };


   template <typename T>
   concept TableData = requires (T t, T t2, int idx, T::value_type::Props prop)
   {
      t.size();
      t[idx][prop];
      t.swap(t2);
   };


   template<TableData Derived>
   class CtGridTable : public wxGridTableBase, protected Derived
   {
   public:
      void loadTable(Derived&& table_data)
      {
         asDerived().swap(table_data);
      }

      int GetNumberRows() override { return rowCount();    }
      int GetNumberCols() override { return columnCount();  }

      wxString GetValue(int row, int col) override
      {
         if (row >= rowCount())
         {
            assert(false);
            return wxString{};
         }

         return asDerived()[static_cast<size_t>(row)][col]
            .and_then([](auto val) -> std::expected<wxString, Error>
               {
                  // todo: check perf, may want to use manual visit
                  return std::visit(FieldToStr, val);
               })
            .value_or(wxString{});
      }

      void SetValue(int row, int col, const wxString& value) override
      {
         throw Error("This dataset does not support editing.");
      }

   private:
      constexpr Derived& asDerived()    
      { 
         return static_cast<Derived&>(*this); 
      }
      
      constexpr const Derived&  asDerived() const
      {
         return static_cast<const Derived&>(*this);
      }

      int rowCount()  const   
      { 
         return std::ssize(asDerived());        
      }

      int columnCount() const 
      { 
         return static_cast<int>(magic_enum::enum_count<Derived::value_type::Props>()); 
      }
   };


   class CtGridTableMgr
   {
   public:
      enum class GridTable
      {
         WineList,
         ReadyToDrinkList,
      };

      /// @brief the smart-ptr-to-base that this class returns to callers.
      using GridTablePtr = std::shared_ptr<wxGridTableBase>;

      /// @brief get the requested grid table
      GridTablePtr getGridTable(GridTable tbl);

   private:
      using GridTables = std::unordered_map<GridTable, GridTablePtr>;
      GridTables m_tables{};
   };


}  // namespace cts::data
