#pragma once


namespace ctb::detail
{

   // unique_ptr-style RAII class for dealing with the various handles used
   // by Windows API's.
   //
   // like unique_ptr, this class supports move semantics but not copy/assignment.
   // The difference is that this class doesn't store a pointer-to-type, because the
   // pointer/handle IS the type. Try to initialize a unique_ptr<HCERTSTORE>
   // and you'll realize the problem. This class also adds get_ptr() so that you can
   // pass ptr-to-handle (or address-of-ptr) to API's requiring it.
   //
   //
   template<typename HandleT, typename FreeFuncT>
   class ScopedHandle
   {
   public:
      ScopedHandle(const ScopedHandle&) = delete;
      explicit ScopedHandle(HandleT h = NULL) noexcept : m_handle(h)   // NOLINT [modernize-use-nullptr] nullptr causes compile issues for some handle types that aren't actually pointers.
      {}

      ScopedHandle(ScopedHandle&& other) noexcept
      {
         *this = std::move(other);
      }
      ScopedHandle& operator=(ScopedHandle& other) = delete;
      ScopedHandle& operator=(ScopedHandle&& other) noexcept
      {
         swap(other);
         return *this;
      }

      ~ScopedHandle() noexcept
      {
         reset();
      }

      void swap(ScopedHandle& other) noexcept
      {
         m_handle = std::exchange(other.m_handle, m_handle);

         // this may seem pointless since m_free_func will usually be a simple
         // fn ptr or functor, but some functors may have state so we should
         // still swap just to be thorough.
         m_free_func = std::exchange(other.m_free_func, m_free_func);
      }

      void reset(HandleT h = NULL) noexcept
      {
         try
         {
            if (m_handle != NULL) m_free_func(m_handle);
         }
         catch (...)   // NOLINT
         {}
         m_handle = h;
      }

      HandleT release() noexcept
      {
         auto rc  = m_handle;
         m_handle = NULL;
         return rc;
      }

      // cppcheck-suppress functionConst; because this handle could be a ptr
      template<typename Self>
      auto get(this Self&& self) noexcept
      {
         return std::forward<Self>(self).m_handle;
      }

      // To be used with API's that take a ptr to handle as an "out" parameter.
      // Any current handle this object is holding will be reset() and set to null
      // before the pointer is returned.
      HandleT* get_ptr() noexcept
      {
         reset();
         return &m_handle;
      }

      explicit operator bool() const noexcept
      {
         return get() != NULL; // NOLINT [modernize-use-nullptr] 
      }

   private:
      HandleT   m_handle{};
      FreeFuncT m_free_func{};
   };

   template<typename HandleT, typename FreeFuncT>
   inline void swap(ScopedHandle<HandleT, FreeFuncT>& lhs, ScopedHandle<HandleT, FreeFuncT>& rhs) noexcept
   {
      lhs.swap(rhs);
   }


}   // namespace ctb::detail
