#pragma once


namespace ctb::detail
{

   // unique_ptr-style RAII class for dealing with the various handles used
   // by Windows API's. See using declarations below for the specific
   // ready-to-use instantiations.
   //
   // like unique_ptr, this class supports move semantics but not copy/assignment.
   // The difference is that this class doesn't store a pointer-to-type, because the
   // pointer/handle IS the type. (Try to initialize a unique_ptr<HCERTSTORE>
   // and you'll realize the problem. This class also adds get_ptr() so that you can
   // pass ptr-to-handle (or addressof-ptr) to API's requiring it.
   //
   template<typename HandleT, typename FreeFuncT>
   class auto_handle
   {
   public:
      auto_handle(const auto_handle&) = delete;
      explicit auto_handle(HandleT h = NULL) noexcept : _h(h)   // nullptr causes compile issues for some handle types.
      {}

      auto_handle(auto_handle&& other) noexcept
      {
         *this = std::move(other);
      }
      auto_handle& operator=(auto_handle& other) = delete;
      auto_handle& operator=(auto_handle&& other) noexcept
      {
         swap(other);
         return *this;
      }

      ~auto_handle() noexcept
      {
         reset();
      }

      void swap(auto_handle& other) noexcept
      {
         _h = std::exchange(other._h, _h);

         // this may seem pointless since _free_func will usually be a simple
         // fn ptr oR functor, but some functors may have state so we should
         // still swap just to be thorough.
         _free_func = std::exchange(other._free_func, _free_func);
      }

      void reset(HandleT h = NULL) noexcept
      {
         try
         {
            if (_h != NULL) _free_func(_h);
         }
         catch (std::exception&)   // NOTLINT (bugprone-empty-catch) since this gets called from dtor
         {}
         _h = h;
      }

      HandleT release() noexcept
      {
         auto rc = _h;
         _h      = NULL;
         return rc;
      }

      // cppcheck-suppress functionConst; because this handle could be a ptr
      HandleT get() noexcept
      {
         return _h;
      }

      // To be used with API's that take a ptr to handle as an "out" parameter.
      // Any current handle this object is holding will be reset() and set to null
      // before the pointer is returned.
      HandleT* get_ptr() noexcept
      {
         reset();
         return &_h;
      }

   private:
      HandleT   _h;
      FreeFuncT _free_func;
   };

   template<typename HandleT, typename FreeFuncT>
   inline void swap(auto_handle<HandleT, FreeFuncT>& lhs, auto_handle<HandleT, FreeFuncT>& rhs)
   {
      lhs.swap(rhs);
   }


}   // namespace ctb::detail
