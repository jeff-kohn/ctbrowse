#pragma once

#include "App.h"
#include "controls/WineDetailFields.h"
#include "model/DatasetWindow.h"

#include <wx/panel.h>

namespace ctb::app
{

   /// @brief  base class used for deriving classes that implement a property panel
   ///         showing an optional title along with a 2-column table of property labels and values.
   /// 
   /// To make use of this class, all the derived class has to do is inherit from WineDetailBasePanel
   /// pass the title (if any) to ctor land implement getDetailFields() to add the property fields.
   /// 
   class WineDetailBasePanel : public DatasetWindow<wxPanel>
   {
   public:
      // no public interface for now.

   protected:
      using Base         = DatasetWindow<wxPanel>;
      using DetailField  = std::variant<SinglePropDetailField, DrinkWindowDetailField>;
      using DetailFields = std::deque<DetailField>;

      /// @brief derived classes must implement this to populate DetailFields with the fields the panel will display.
      virtual void getDetailFields(DetailFields& fields) = 0;

      /// @brief derived classes can override this to bind message handlers or carry out any other setup that requires
      ///        a valid window object. 
      /// 
      /// This is called after createWindow() has done initial setup including calling getDetailFields().
      virtual void postWindowCreate() {} 

      /// @brief event handler for dataset events. 
      /// 
      /// Default processing should be sufficient for most classes but this can be overrridden if necessary, and calling
      /// this version from derived version is fine/recommended.
      virtual void onDatasetEvent(const DatasetEvent& event);

      /// @brief protectec ctor
      ///
      /// empty title will not be displayed.
      WineDetailBasePanel(const DatasetEventSourcePtr& event_source, std::string_view title = "") :
         Base{ event_source },
         m_title(wxFromSV(title))
      {}

      void createWindow(wxWindow* parent) override;

   private:
      DetailFields        m_fields{};
      wxString            m_title{};
   };

} // namespace ctb::app