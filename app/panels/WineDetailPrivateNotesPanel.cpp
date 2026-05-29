#include "WineDetailPrivateNotesPanel.h"
#include "controls/ElasticMultiLineTextCtrl.h"

#include <ctb/tables/CtSchema.h>

#include <wx/sizer.h>

namespace ctb::app
{

   void WineDetailPrivateNotesPanel::postWindowCreate()
   {
      auto source = getEventHandler().getSource();
      auto* top_sizer = GetSizer(); assert(top_sizer);

      // private note
      auto* note_ctrl = ElasticMultiTextCtrl::create(this, source, CtProp::PrivateNote);
      top_sizer->Add(note_ctrl, wxSizerFlags{2}.Expand().TripleBorder());
   }


} // namespace ctb::app
