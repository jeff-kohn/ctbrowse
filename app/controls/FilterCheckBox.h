#pragma once
#include "App.h"

#include <ctb/tables/CtSchema.h>
#include <wx/checkbox.h>
#include <wx/valgen.h>

namespace ctb::app
{

   /// @brief Really simple control class that binds a CtPropertyFilter to a checkbox control
   class FilterCheckBox final : public wxCheckBox
   {
   public:
      using PropertyFilter = CtPropertyFilter;
      static auto create(wxWindow* parent, const PropertyFilter& filter) -> FilterCheckBox*
      {
         if (!parent)
         {
            assert("FilterCheckBox parent pointer cannot == nullptr");
            throw Error{ Error::Category::ArgumentError, constants::ERROR_STR_NULLPTR_ARG };
         }

         std::unique_ptr<FilterCheckBox> wnd{ new FilterCheckBox{ filter } };
         wnd->createWindow(parent);
         return wnd.release(); // if we get here parent owns it, so return non-owning*
      }


      /// @brief Get a reference to the filter associated with this control
      /// @return reference to the filter, which will have the appropriate cvref corresponding to 'this'
      template<typename Self>
      auto&& filter(this Self&& self)
      {
         return std::forward<Self>(self).m_filter;
      }

      auto enabled() const -> bool
      {
         return m_filter_enabled;
      }

      void enable(bool enable)
      {
         m_filter_enabled = enable;
         TransferDataToWindow();
      }

      // no copy/move/assign, this class is created on the heap and passed around as ptr
      FilterCheckBox(const FilterCheckBox&) = delete;
      FilterCheckBox(FilterCheckBox&&) = delete;
      FilterCheckBox& operator=(const FilterCheckBox&) = delete;
      FilterCheckBox& operator=(FilterCheckBox&&) = delete;
      ~FilterCheckBox() noexcept override = default;

   private:
      PropertyFilter  m_filter{};
      bool            m_filter_enabled{false};

      FilterCheckBox(PropertyFilter filter) : m_filter{ std::move(filter) }
      {}

      void createWindow(wxWindow* parent)
      {
         if (!Create(parent, wxID_ANY, wxFromSV(m_filter.filter_name)))
         {
            throw Error{ Error::Category::UiError, constants::ERROR_WINDOW_CREATION_FAILED };
         }
         SetValidator(wxGenericValidator{ &m_filter_enabled });
      }
   };

};