/*******************************************************************
 * @file GridOptionsPanel.cpp
 *
 * @brief implementation file for the GridOptionsPanel class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/

#include "App.h"
#include "panels/GridOptionsPanel.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/valgen.h>

#include <memory>

namespace ctb::app
{
   
   GridOptionsPanel::GridOptionsPanel(GridTableEventSourcePtr source) : m_sink{ this, source }
   {}


   [[nodiscard]] GridOptionsPanel* GridOptionsPanel::create(wxWindow* parent, GridTableEventSourcePtr source)
   {
      if (!source)
      {
         assert("source parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }
      if (!parent)
      {
         assert("parent parameter cannot == nullptr");
         throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
      }

      std::unique_ptr<GridOptionsPanel> wnd{ new GridOptionsPanel{source} };
      if (!wnd->Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME))
      {
         throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
      }
      wnd->initControls();
      return wnd.release(); // parent owns child, so we don't need to delete
   }


   void GridOptionsPanel::initControls()
   {
      // panel shouldn't grow infinitely
      SetMaxSize(ConvertDialogToPixels(wxSize{ 140, constants::UNSPECIFIED }));
      SetMinSize(ConvertDialogToPixels(wxSize{ 70, constants::UNSPECIFIED }));

      // defines the rows of controls in our panel
      auto top_sizer = new wxBoxSizer{ wxVERTICAL };

      // sort field label
      auto* static_text = new wxStaticText(this, wxID_ANY, constants::LBL_SORT_BY);
      top_sizer->Add(static_text, wxSizerFlags{}.Border(wxTOP | wxLEFT, wxSizerFlags::GetDefaultBorder() * 1.5));

      // sort fields combo
      m_sort_combo = new wxChoice(this, wxID_ANY);
      m_sort_combo->SetFocus();
      m_sort_combo->SetValidator(wxGenericValidator(&m_sort_config.sort_index));
      top_sizer->Add(m_sort_combo, wxSizerFlags{}.Expand().Border(wxLEFT | wxRIGHT ));
      top_sizer->AddSpacer(5);

      // sort order radio group
      auto sort_order_group = new wxStaticBoxSizer{ wxHORIZONTAL, this, constants::LBL_SORT_ORDER };

      // ascending sort order radio. validator tied to SortConfig.ascending
      auto opt_ascending = new wxRadioButton{
         sort_order_group->GetStaticBox(),
         wxID_ANY, 
         constants::LBL_SORT_ASCENDING,
         wxDefaultPosition, 
         wxDefaultSize, 
         wxRB_GROUP
      };
      opt_ascending->SetValue(true);
      opt_ascending->SetValidator(wxGenericValidator{ &m_sort_config.ascending });
      sort_order_group->Add(opt_ascending, wxSizerFlags{1}.Expand().Border(wxALL));

      // descending sort order radio. no validator needed 
      auto opt_descending = new wxRadioButton{ sort_order_group->GetStaticBox(), wxID_ANY, constants::LBL_SORT_DESCENDING };
      sort_order_group->Add(opt_descending, wxSizerFlags{1}.Expand().Border(wxALL));

      // finalize layout
      top_sizer->Add(sort_order_group, wxSizerFlags().Expand().Border(wxALL));
      SetSizer(top_sizer);

      // event bindings.
      m_sort_combo->Bind(wxEVT_CHOICE, &GridOptionsPanel::onSortSelection, this);
      opt_ascending->Bind(wxEVT_RADIOBUTTON, &GridOptionsPanel::onSortOrderClicked, this);
      opt_descending->Bind(wxEVT_RADIOBUTTON, &GridOptionsPanel::onSortOrderClicked, this);

   }

   wxArrayString GridOptionsPanel::getSortOptionList(GridTable* grid_table)
   {
      using SortConfig = GridTable::SortConfig;

      return vws::all(grid_table->availableSortConfigs()) 
         | vws::transform([](const SortConfig& s) {  return wxString{s.sort_name.data(), s.sort_name.length() };  })
         | rng::to<wxArrayString>();
   }


   void GridOptionsPanel::onTableInitialize(GridTable* grid_table)
   {
      // load the correct sort options into the combo, and select the active one.
      m_sort_combo->Clear();
      m_sort_combo->Append(getSortOptionList(grid_table));
      onTableSorted(grid_table);
   }

   void GridOptionsPanel::onTableSorted(GridTable* grid_table)
   {
      m_sort_config = grid_table->activeSortConfig();
      TransferDataToWindow();
   }


   void GridOptionsPanel::onSortSelection([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         TransferDataFromWindow();

         // let the combo close its list before we reload the grid
         CallAfter([this](){
               auto table = m_sink.getTable();
               if (table)
               {
                  table->setActiveSortConfig(m_sort_config);
                  m_sink.signal_source(GridTableEvent::Sort);
               }
         });
      }
      catch(Error& err)
      {
         wxGetApp().displayErrorMessage(err);
      }
      catch(std::exception& e)
      {
         wxGetApp().displayErrorMessage(e.what());
      }
   }

   
   void GridOptionsPanel::onSortOrderClicked([[maybe_unused]] wxCommandEvent& event)
   {
      try
      {
         TransferDataFromWindow();
         auto table = m_sink.getTable();
         if (table)
         {
            table->setActiveSortConfig(m_sort_config);
            m_sink.signal_source(GridTableEvent::Sort);
         }
      }
      catch(Error& err)
      {
         wxGetApp().displayErrorMessage(err);
      }
      catch(std::exception& e)
      {
         wxGetApp().displayErrorMessage(e.what());
      }

   }


   void GridOptionsPanel::notify(GridTableEvent event, GridTable* grid_table)
   {
      switch (event)
      {
         case GridTableEvent::TableInitialize:
            onTableInitialize(grid_table);
            break;

         case GridTableEvent::Sort:
            onTableSorted(grid_table);
            break;

         case GridTableEvent::Filter:
         case GridTableEvent::SubStringFilter:
         case GridTableEvent::RowSelected:
         default:
            break;
      }   
   }


} // namespace ctb::app