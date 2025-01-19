#pragma once
#include "wx/grid.h"

namespace cts::ui
{
   class CtGrid final : public wxGrid
   {
      CtGrid() = default;
      CtGrid(wxWindow* parent) : wxGrid{ parent };
   };

};
