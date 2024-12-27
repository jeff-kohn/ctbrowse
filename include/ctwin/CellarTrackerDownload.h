///
/// @file CellarTrackerDownload.h
/// 
/// @brief this header contains declaration for the CellarTrackerDownload class.
/// 
#pragma once

#include <string>
#include <string_view>
#include <expected>

namespace ctwin
{

   ///
   /// @brief CellarTrackerDownload class retrieves user data from the CellarTracker website.
   /// 
   /// This class downloads table data from the CT website using HTTP requests. Username/passwword
   /// authentication is used. Supports getting password from an environment variable.
   ///  
   class CellarTrackerDownload
   {
   public:
      
      CellarTrackerDownload() = default;
      CellarTrackerDownload(std::string_view user_name, std::string_view pwd);


      /// @brief enum for the data tables available from CT website
      enum class Table
      {
         List,			   /// Wine Summary (does not include location or bin unless optional parameter Location=1)
         Inventory,		/// Individual Bottles
         Notes,			/// Tasting Notes
         PrivateNotes,	/// Private Notes
         Purchase,	   /// Purchases
         Pending,	      /// Pending Purchases(Futures)
         Consumed,	   /// Consumed Bottles
         Availability,	/// Ready to Drink(Drinkability) report
         Tag,	         /// Wishlists
         ProReview,	   /// Your manually - entered Professional Reviews
         Bottles,	      /// A special raw view showing all bottles with a BottleState parameter(-1 for pending, 1 for in - stock, 0 for consumed)
         FoodTags	      /// Your food pairing tags
      };


      /// @brief enum for available data formats
      enum class Format
      {
         html,	// default if not specified
         xml,
         tab,
         csv
      };


      /// @brief 
      /// @param user_name CT username
      /// @param pwd  CT password, or an environment variable containing the password.
      void setCredentials(std::string_view user_name, std::string_view pwd);

      /// @brief  retreive a data table from CT website
      /// @param tbl the table to retrieve
      /// @param fmt the data format to return1
      /// @return 
      std::string getTableData(Table tbl, Format fmt);
      bool 

   private:
      std::string m_user{};
      std::string m_pwd{};
   };

} // namespace ctwin
