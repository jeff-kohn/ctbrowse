#include "App.h"
#include "controls/WineDetailFields.h"

#include <ctb/model/DatasetEventHandler.h>
#include <wx/panel.h>
#include <deque>

namespace ctb::app
{
   /// @brief A wxPanel-derived class that displays details about a wine, handling dataset events and rendering relevant fields.
   ///
   class WineDetailTagsPanel final : public wxPanel
   {
   public:
      WineDetailTagsPanel(wxWindow* parent, DatasetEventSourcePtr event_source);

   private:
      using DetailFields = std::deque<SinglePropDetailField>;

      DatasetEventHandler m_event_handler;
      DetailFields        m_fields{};
      wxString            m_tag_note{};
      wxStaticText*       m_tag_note_ctrl{};

      void init();
      void onDatasetEvent(const DatasetEvent& event);
   };
}