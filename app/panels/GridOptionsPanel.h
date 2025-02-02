/*******************************************************************
 * @file GridOptionsPanel.h
 *
 * @brief Header file for GridOptionsPanel class
 * 
 * @copyright Copyright © 2025 Jeff Kohn. All rights reserved. 
 *******************************************************************/
#pragma once
#include "grids/GridTableBase.h"

#include <wx/choice.h>
#include <wx/gdicmn.h>
#include <wx/panel.h>


namespace ctb::app
{
   // we need a ptr to parent window
   class MainFrame;


   /// @brief panel class that provides UI for sorting and filtering a grid
   class GridOptionsPanel final : public wxPanel
   {
   public:
      GridOptionsPanel() = default;
      GridOptionsPanel(MainFrame* parent);

      bool Create(MainFrame* parent);

      void populateSortOptions(GridTableBase::GridTablePtr grid);

   private:
      MainFrame* m_parent{};
      int        m_sort_idx{};
      wxChoice*  m_sort_combo{}; 
   };

} // namespace ctb::app