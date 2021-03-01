#pragma once

#include <utility>
#include <type_traits>

namespace pc
{
   template <typename F>
   // Change Requires to an ugly enable_if_t or a static_assert for C++17 support
   requires(std::is_invocable_v<F>) struct scoped_exit
   {
      // Store the function only
      // No need to store a 2nd variable
      F f;
      scoped_exit(F&& f) : f(std::forward<F>(f)) {}

      ~scoped_exit() noexcept(
          // GSL Destructors are always noexcept
          // But sometimes, we might have to throw an exception
          // It is a convenience factor
          std::is_nothrow_invocable_v<F>)
      {
         f();
      }

      // Disable move & copy
      scoped_exit(const scoped_exit&) = delete;
      scoped_exit(scoped_exit&&)      = delete;
      scoped_exit& operator=(scoped_exit&&) = delete;
      scoped_exit& operator=(const scoped_exit&) = delete;
   };

   // Ensure naming convention matches GSL
   // To make switching from GSL to pc::scoped_exit easy
   template <typename F>
   using finally = scoped_exit<F>;
   template <typename F>
   using final_action = scoped_exit<F>;
} // namespace pc