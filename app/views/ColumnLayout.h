#pragma once

#include "App.h"

#include <glaze/glaze.hpp>

namespace ctb::app
{

   struct ColumnLayout
   { 
      static inline constexpr uint32_t DEFAULT_WIDTH = 50;

      int  width{ DEFAULT_WIDTH };
      bool bold_text { false };
   };


   /// @brief concept for array-like container of ColumnLayout structs.
   ///
   /// Can be used to serialize collection of ColumnLayouts to config store
   /// 
   //template <typename ContainerT>
   //concept ColumnLayoutContainer = requires (ContainerT cont1, ContainerT cont2, ColumnLayout cl)
   //{
   //   cont1.emplace_back(cl);
   //   cont2[0] = cl;
   //   cont1.swap(cont2);
   //};


   class ColumnLayouts final : public std::vector<ColumnLayout> 
   {
   public:
      static inline constexpr const char* CONFIG_PATH = "/Preferences/GridLayouts";

      using ContainerBase = std::vector<ColumnLayout>;
       
      /// @brief Load a collection of column layouts from the config store
      /// @param table_name - name of the grid table to load layouts for
      /// @return true if successful, false if not in which case this container will be unchanged
      /// (strong exception guarantee)
      /// 
      auto loadConfig(const std::string_view table_name) -> bool
      {
         try
         {
            auto cfg = wxGetApp().getConfig();
            cfg->SetPath(CONFIG_PATH);
            auto json_str = cfg->Read(wxString{ table_name.data(), table_name.size() }, wxEmptyString);
            if (json_str.empty())
               throw ctb::Error(Error::Category::UiError,  "No saved column layouts found.");

            auto cols = glz::read_json<ContainerBase>(json_str.ToStdString());
            if (cols and cols->size() > 0)
            {
               this->swap(cols.value());
               return true;
            }
         }
         catch (...)
         {
            auto e = packageError();
            log::info("Failed to load column layouts for table {}. {}", table_name, e.formattedMesage());
         }
         return false;
      }

      auto saveConfig(const std::string_view table_name) -> bool
      {
         try
         {
            auto json_result = glz::write_json(*this);
            if (!json_result)
               throw ctb::Error(Error::Category::UiError,  "Couldn't write column layouts to json. {}", json_result.error().custom_error_message);

            auto cfg = wxGetApp().getConfig();
            cfg->SetPath(CONFIG_PATH);
            cfg->Write(wxString{ table_name.data(), table_name.size() }, json_result->c_str());
            return true;
         }
         catch (...)
         {
            auto e = packageError();
            log::info("Failed to save column layouts for table {}. {}", table_name, e.formattedMesage());
         }
         return false;
      }
   };


} // namespace ctb::app