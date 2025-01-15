/*********************************************************************
 * @file       TableSyncDialog.cpp
 *
 * @brief      Implementation for the class TableSyncDialog
 *
 * @copyright  Copyright © 2025 Jeff Kohn. All rights reserved.
 *********************************************************************/

#include "App.h"
#include "TableSyncDialog.h"
#include "wx_helpers.h"
#include "concepts.h"

#include "cts/CellarTrackerDownload.h"

#include <magic_enum/magic_enum.hpp>

#include <algorithm>
#include <sstream>

namespace cts
{
   using namespace magic_enum;

   inline constexpr auto ENUM_DELIMETER = ';';

   namespace {

      /// @brief serialize a range of integer values to a delimited string
      template<rng::input_range Rng, typename I = rng::range_value_t<Rng> >
         requires std::is_same_v<std::decay_t<rng::range_value_t<Rng>>, I>
      std::string serializeIntegers(Rng rg, char delim = ENUM_DELIMETER)
      {
         std::ostringstream str{};
         for (I i : rg)
         {
            str << i << delim;
         }
         return str.str();
      }
   }

   /// @brief  user-friendly version of from_chars that works with string_view and string
   /// @return an optional containing the requested value if successful, or an empty optional otherwise.
   template<typename T, StringViewCompatible S>
   std::optional<T> from_str(S str)
   {
      T val{};
      auto result = std::from_chars(str.data(), str.data() + str.size(), val);

      if (result.ec != std::errc())
         return {};  // there was an error, return uninitialized optional

      return val;
   }


   TableSyncDialog::TableSyncDialog(wxWindow* parent)
   {
      Create(parent);
   }


   bool TableSyncDialog::Create(wxWindow* parent)
   {
      // give base class a chance set up controls etc
      if (!TableSyncDlgBase::Create(parent))
         return false;

      // message handlers
      Bind(wxEVT_UPDATE_UI, &TableSyncDialog::onOkUpdateUI, this, wxID_OK);
      Bind(wxEVT_BUTTON, &TableSyncDialog::onOkClicked, this, wxID_OK);

      // populate table name selection list
      constexpr auto table_descriptions = vws::all(data::CellarTrackerDownload::tableDescriptions()) | vws::values;
      m_table_selection_ctrl->InsertItems(wxToArrayString(table_descriptions), 0);

      // need to read some defaults from config settings.
      auto& cfg = wxGetApp().getConfig();
      cfg.SetPath(constants::CONFIG_PATH_SYNC);

      // default-selected tables are stored as a string of enum values (e.g int values not names)
      // delimited by ENUM_DELIMTER. The default value is the table enum value 0 (List)
      m_table_selection_val = std::string_view{ cfg.Read(constants::CONFIG_VALUE_DEFAULT_SYNC_TABLES, "0").wx_str() } // read the config value
         | vws::split(ENUM_DELIMETER)                                                                                 // split by token ';'
         | vws::transform([] (auto subrange) { return std::string_view(subrange.begin(), subrange.end()); })          // convert subranges to string_view's
         | vws::transform([] (std::string_view sv) { return from_str<int>(sv); })                                     // convert string view to from_chars() result
         | vws::filter([] (auto opt) { return opt.has_value(); })                                                     // filter out results that have no value
         | vws::transform([](auto opt) { return opt.value(); })                                                       // retrieve actual value from remaining results
         | rng::to<wxArrayInt>();                                                                                     // convert to array

      // whether the "Sync on Startup" box should be checked.
      m_startup_sync_val = cfg.ReadBool(constants::CONFIG_VALUE_SYNC_ON_STARTUP, false);

      TransferDataToWindow();
      return true;
   }


   std::vector<data::TableId> TableSyncDialog::selectedTables() const
   {
      using namespace vws;
      using EnumT = data::TableId;

      // convert our internal list of ints to the actual enum, need to filter out
      // values that don't map to an enum (should never happen, but best to be
      // prepared since alternative is UB)
      return all(m_table_selection_val)
         | transform([] (int val) { return enum_cast<EnumT>(val); }) // convert to optional<EnumT>
         | filter([](auto maybe_enum) { return maybe_enum.has_value(); })        // filter only valid values
         | transform([](auto maybe_enum) { return maybe_enum.value(); })         // retrieve actual value from optionals
         | rng::to<std::vector>();
   }


   void TableSyncDialog::onOkClicked(wxCommandEvent& event)
   {
      if (!TransferDataFromWindow())
         wxMessageBox(constants::ERROR_DIALOG_TRANSFER_FAILED,
                      constants::ERROR_STR,
                      wxOK | wxICON_ERROR | wxCENTRE, this);

      // Save relevant settings to config
      auto& cfg = wxGetApp().getConfig();
      cfg.SetPath(constants::CONFIG_PATH_SYNC);
      cfg.Write(wxString(constants::CONFIG_VALUE_SYNC_ON_STARTUP), m_startup_sync_val);
      if (m_save_default_val)
      {
         cfg.Write(wxString(constants::CONFIG_VALUE_DEFAULT_SYNC_TABLES), wxString(serializeIntegers(vws::all(m_table_selection_val))));
      }
      cfg.Flush();

      EndDialog(wxID_OK);
   }

   void TableSyncDialog::onDeselectAll(wxCommandEvent& event)
   {
      for (auto idx = 0u; idx < m_table_selection_ctrl->GetCount(); ++idx)
      {
         m_table_selection_ctrl->Check(idx, false);
      }
   }


   void TableSyncDialog::onDeselectAllUpdateUI(wxUpdateUIEvent & event)
   {
      // we already check for at least one selection for OK button, so just piggy back.
      onOkUpdateUI(event);
   }


   void TableSyncDialog::onSelectAll(wxCommandEvent & event)
   {
      for (auto idx = 0u; idx < m_table_selection_ctrl->GetCount(); ++idx)
      {
         m_table_selection_ctrl->Check(idx, true);
      }
   }


   void TableSyncDialog::onSelectAllUpdateUI(wxUpdateUIEvent & event)
   {
      event.Enable(checkedTableCount() != m_table_selection_ctrl->GetCount());
   }


   void TableSyncDialog::onOkUpdateUI([[maybe_unused]] wxUpdateUIEvent& event)
   {
      // only enabled if at least one is checked
      event.Enable(checkedTableCount());
   }


}  // namespace cts
