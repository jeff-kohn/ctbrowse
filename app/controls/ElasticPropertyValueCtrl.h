#pragma once

#include "App.h"
#include "model/DatasetWindow.h"

#include <wx/textctrl.h>

namespace ctb::app
{

   /// @brief base class used for our property value controls.
   ///
   /// This control displays a formatted single-line value in a read-only text control that looks
   /// like a static text control but supports selecting/copying text. The control will dynamically
   /// expand its width to accommodate changing values just like a static text ctrl.
   /// 
   class ElasticPropertyValueCtrl : public DatasetWindow<wxTextCtrl>
   {
   protected:
      ElasticPropertyValueCtrl(const DatasetEventSourcePtr& event_source);

      /// @brief retrieve the value to be displayed in the control from the dataset
      ///
      /// derived classes must implement this so that the control knows what to display
      /// 
      /// @return formatted display string for the property value
      virtual auto getDisplayValue(const DatasetPtr& ds, int rec_idx) const -> std::string = 0;

   private:
      wxString m_display_value{};

      DECLARE_DATASET_WINDOW_FACTORY;

      void createWindow(wxWindow* parent) override;
      void onDatasetRowSelected(const DatasetEvent& event);
   };


   /// @brief standard/default property control that displays a single property
   ///
   class PropertyValueCtrl : public ElasticPropertyValueCtrl
   {
   public:
      /// @brief static factory method for creating PropertyValueCtrl objects
      /// @param parent parent window for the control, must be non-null
      /// @param source dataset event source to bind the control to, must be non-null
      /// @param bound_prop the property id to bind the control to
      /// @return non-owning pointer to the newly created window.
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source, CtProp bound_prop) -> PropertyValueCtrl*;

      /// @brief Set the display format.
      ///
      /// Default is "{}" which just displays the string property, but you can change it if needed (e.g. currency etc)
      template<typename Self>
      auto setFormat(this Self&& self, std::string_view fmt_str)
      {
         self.m_format_str = fmt_str;
         return std::forward<Self>(self);
      }

      /// @brief Set the value to display when the bound field isNull(). Default is empty string
      template<typename Self>
      auto setNullDisplayValue(this Self&& self, std::string_view val)
      {
         self.m_null_display = val;
         return std::forward<Self>(self);
      }

   private:
      CtProp m_prop{};
      std::string m_format_str{ constants::FMT_DEFAULT_FORMAT };
      std::string m_null_display{};

      DECLARE_DATASET_WINDOW_FACTORY;

      PropertyValueCtrl(const DatasetEventSourcePtr& source, CtProp bound_prop) : ElasticPropertyValueCtrl{ source }, m_prop{ bound_prop }
      {}

      auto getDisplayValue(const DatasetPtr& ds, int rec_idx) const -> std::string override;

   };


   /// @brief This class uses drink start/end properties to display a formatted drinking window.
   ///
   class DrinkWindowCtrl : public ElasticPropertyValueCtrl
   {
   public:
      /// @brief static factory method for creating DrinkWindowCtrl objects
      /// @param parent parent window for the control, must be non-null
      /// @param source dataset event source to bind the control to, must be non-null
      /// @param bound_prop the property id to bind the control to
      /// @return non-owning pointer to the newly created window.
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source, CtProp begin_prop, CtProp end_prop) -> DrinkWindowCtrl*;

   private:
      CtProp m_begin_prop{};
      CtProp m_end_prop{};

      DrinkWindowCtrl(const DatasetEventSourcePtr& source, CtProp begin_prop, CtProp end_prop)
         : ElasticPropertyValueCtrl{ source }, m_begin_prop{ begin_prop }, m_end_prop{ end_prop }
      {}

      auto getDisplayValue(const DatasetPtr& ds, int rec_idx) const -> std::string override;

      DECLARE_DATASET_WINDOW_FACTORY;
   };


   /// @brief This class binds a property value control to one of the getXXX() methods in the ProReviewsCache class.
   ///
   class ProReviewsCacheCtrl final : public ElasticPropertyValueCtrl
   {
   public:
      using CacheValueFn = std::string (ProReviewsCache::*)(uint64_t) const;

      /// @brief static factory method for creating ProReviewsCacheCtrl objects
      /// @param parent the parent window for the control, must be non-null
      /// @param source the dataset event source to bind the control to, must be non-null
      /// @param bound_prop the property id to bind the control to
      /// @return non-owning pointer to the newly created window.
      [[nodiscard]] static auto create(wxWindow* parent, const DatasetEventSourcePtr& source, CacheValueFn value_fn) -> ProReviewsCacheCtrl*;


   private:
      CacheValueFn m_value_fn{};

      ProReviewsCacheCtrl(const DatasetEventSourcePtr& source, CacheValueFn value_fn) : ElasticPropertyValueCtrl{ source }, m_value_fn{ std::move(value_fn) }
      {}

      auto getDisplayValue(const DatasetPtr& ds, int rec_idx) const -> std::string override;

      DECLARE_DATASET_WINDOW_FACTORY;
   };


}   // namespace ctb::app
