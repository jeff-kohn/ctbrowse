#pragma once

#include "App.h"

#include <ctb/model/DatasetEventHandler.h>
#include <wx/window.h>

#include <concepts>
#include <memory>
#include <utility>

namespace ctb::app
{
   
   /// @brief Base-class  A base-class for wxPanel-derived classes that are dataset-aware.
   ///
   template <typename BaseWindowT> requires std::derived_from<BaseWindowT, wxWindow>
   class DatasetWindow : public BaseWindowT 
   {
   public:
      // no copy/move/assign, this class is created on the heap and shouldn't be copied.
      DatasetWindow(const DatasetWindow&) = delete;
      DatasetWindow(DatasetWindow&&) = delete;
      DatasetWindow& operator=(const DatasetWindow&) = delete;
      DatasetWindow& operator=(DatasetWindow&&) = delete;
      ~DatasetWindow() override = default;

   protected:
      DatasetWindow(const DatasetEventSourcePtr& event_source) : m_dataset_events{ event_source }
      {}

      /// @brief returns a reference to the event source. 
      template<typename Self>
      auto&& getEventHandler(this Self&& self) noexcept(true)
      {
         return std::forward<Self>(self).m_dataset_events;
      }

      /// @brief returns reference to the DatasetPtr for our event source. Throws exception if 
      ///        source has no Dataset. You can call getEventHandler().getDataset(false) if you want
      ///        to do your own nullptr check.
      template<typename Self>
      auto getDataset(this Self&& self) noexcept(false)
      {
         return std::forward<Self>(self).getEventHandler().getDataset(true);
      }

      /// @brief create the UI window object. Must be overridden by derived classes
      /// @param parent - the parent window for this UI control. will be non-null.
      virtual void createWindow(wxWindow* parent) = 0;

   private:
      DatasetEventHandler m_dataset_events;
   };


   namespace detail
   {
      // helper function that DatasetWindow-derived classes. If derived contructor is private
      // (as it should be to prevent stack-based instances), you'll need to delcare this function 
      // a friend to use it.
      template<typename WndT, typename... Args>
      auto createDatasetWindow(wxWindow* parent, const DatasetEventSourcePtr& source, Args... args) -> WndT*
      {
         if (!parent)
         {
            assert("parent parameter cannot == nullptr" and false);
            throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
         }
         if (!source)
         {
            assert("source parameter cannot == nullptr" and false);
            throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
         }

         std::unique_ptr<WndT> wnd{ new WndT{ source, std::forward<Args>(args)... } };
         wnd->createWindow(parent);
         return wnd.release();
      }

   } // namespace detail

} // namespace ctb::app