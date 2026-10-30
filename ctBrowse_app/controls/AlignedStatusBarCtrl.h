#pragma once

#include "App.h"
#include <wx/statusbr.h>
#include <wx/stattext.h>

namespace ctb::app
{

   /// @brief class AlignedStatusBarCtrl
   ///
   /// Since the lame-ass wxWidgets control doesn't support alignment, we have to
   /// write our own control. So stupid.
   /// 
   class AlignedStatusBarCtrl : public wxStatusBarCtrl
   {
   public:

   };


} // namespace ctb::app
