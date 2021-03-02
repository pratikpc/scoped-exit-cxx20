# Scoped Exit C++20

A better version of [Microsoft GSL](https://github.com/microsoft/GSL)'s [Finally and final_action](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#gslutil-utilities) utilizing C++17 (and [requires](https://en.cppreference.com/w/cpp/keyword/requires) from C++20. Although can easily be replaced by [std::enable_if_t](https://en.cppreference.com/w/cpp/types/enable_if))

---

## What are the differences?

1. Microsoft's implementation supports C++14.  
    Our's does not.  
    All of our advantages can be simplified as, we do not support C++14  
    If your codebase needs to _support C++14_, please **use** Microsoft GSL
2. C++17 introduces [CTAD](https://en.cppreference.com/w/cpp/language/class_template_argument_deduction) which means the `finally` helper is no longer required
3. `final_action` contains a variable `invoke_` which, I don't think is very useful
4. `final_action` can also be moved  
   The only usecase of a movable `final_action` is to make it possible for `finally` to return
6. With [CTAD](https://en.cppreference.com/w/cpp/language/class_template_argument_deduction), that use case is outdated
7. The destructor is always noexcept  
   This seems like a good idea at first  
   But then, we must always realize that the scoped_exit is not always called in the main function  
   And sometimes, even the best of Cleanup codes throw  
   On Cleanup being called during a stack unwinding after an exception throw, we us `std::uncaught_exceptions` to check if there are any exceptions propagating  
   And disable a call to cleanup  
   If there are low chances of this happening, we can disable this  
   You only pay for what you use  
8. At the same time, we need to ensure that, genuine users who need noexcept support should also receive it?
9. This is easily solved with [std::is_nothrow_invocable](https://en.cppreference.com/w/cpp/types/is_invocable)   
   Microsoft's GSL is not using this because this was introduced in C++17

---

## [Usage](sample/run.cxx)

### No Throw

```cpp
#include <scoped_exit.hxx>
#include <iostream>

void no_throw()
{
   // Note that as the Function provided here is noexcept
   // So will our destructor

   // Note that throwing in a nothrow lambda
   // Will lead to noexcept semantics
   pc::scoped_exit const nothrow{
       []() noexcept { std::cout << "Perform cleanup here\n"; }};
   std::cout << "Function Started\n";
   std::cout << "Function Ended\n";
}
```

### Throw

```cpp
#include <scoped_exit.hxx>
#include <iostream>

void throw_but_catch()
{
   try
   {
      // GSL destructor is noexcept
      // But our cleanup may throw an exception
      // Which we might catch later

      pc::scoped_exit const throws{[]() noexcept(false) {
         std::cout << "During Cleanup, we had to throw\n";
         throw std::runtime_error("Sorry. Something during Cleanup errored out");
      }};
      std::cout << "Block Started\n";
      std::cout << "Block Ended\n";
      // Destructor called here
   }
   catch (std::runtime_error const& ex)
   {
      std::cout << "Exception caught : " << ex.what() << "\n";
   }
}
```

### Outer throw leads to cleanup not being called

```cpp
// The cleanup will not be called due to the exception
void throw_outside_cleanup_not_called()
{
   try
   {
      // GSL destructor is noexcept
      // But our cleanup may throw an exception
      // Which we might catch later

      pc::scoped_exit const canThrow{
          []() {
             std::cout << "As there was an exception outside, this was not called\n";
          }};
      std::cout << "Block Started\n";
      throw std::runtime_error("We threw an exception. As there is an exception present, cleanup won't be called");
      std::cout << "Block Ended\n";
      // Destructor called here
   }
   catch (std::runtime_error const& ex)
   {
      std::cout << "Exception caught : " << ex.what() << "\n";
   }
}
```

### Don't check for uncaught exceptions in case chances of them being thrown are low

```cpp
// Let us assume that there is a very low risk of the rest of our code throwing
// But our cleanup might certainly throw
// One way to handle this would be to
// Use the default method
// But the major issue with the default method is
// It's more safer and checks for uncaught_exceptions
// However if you wish to disable the check, you can use ignore_uncaught_exceptions
void dont_check_for_uncaught_exceptions()
{
   try
   {
      // GSL destructor is noexcept
      // But our cleanup may throw an exception
      // Which we might catch later

      pc::scoped_exit const throws{
          []() {
             std::cout << "During Cleanup, we had to throw\n";
             throw std::runtime_error(
                 "As an exception throw outside was unlikely, we disabled the check. It "
                 "worked. We threw. Outside did not");
          },
          pc::dont_check_for_uncaught_exceptions};
      std::cout << "Block Started\n";
      std::cout << "Block Ended\n";
      // Destructor called here
   }
   catch (std::runtime_error const& ex)
   {
      std::cout << "Exception caught : " << ex.what() << "\n";
   }
}
```

---

## [Our code](header/scoped_exit.hxx)

```cpp
#pragma once

#include <exception>
#include <type_traits>
#include <utility>

namespace pc
{
   // Let us say that our F can throw
   // Then in that case, our code will check for uncaught exceptions
   // However, if you are sure outer scope will not throw at all
   // Then you can reduce the cost of the check
   auto inline constexpr dont_check_for_uncaught_exceptions = std::false_type{};

   template <typename F, bool checkForUncaughtExceptions = true>
   // Change Requires to an ugly enable_if_t or a static_assert for C++17 support
   requires(std::is_invocable_v<F>) struct scoped_exit
   {
      // Store the function only
      // No need to store a 2nd variable
      F const f;

      constexpr explicit scoped_exit(
          F&& f,
          // By default check for uncaught exceptions
          // If the Cleanup function is called during
          // An uncaught exception
          // This would ensure that the cleanup is not run
          // However, in case we wish to disable this check at compile-time
          // We can
          const std::bool_constant<checkForUncaughtExceptions> =
              std::true_type{}) noexcept
          // If the function is nothrow invocable
          // Then modifying the checkForUncaughtExceptions is useless
          // So don't let user modify the param
          requires(!(!checkForUncaughtExceptions && std::is_nothrow_invocable_v<F>)) :
          f(std::forward<F>(f))
      {
      }

      ~scoped_exit() noexcept(
          // GSL Destructors are always noexcept
          // But sometimes, we might have to throw an exception
          // It is a convenience factor
          std::is_nothrow_invocable_v<F>)
      {
         // Check if can be invoked without throwing
         // Or user is sure, the chances of a double throw are low
         // Risky. As such checkForUncaughtExceptions is set to true by default
         if constexpr (std::is_nothrow_invocable_v<F> || !checkForUncaughtExceptions)
            f();
         else
             // Run cleanup only when there are no uncaught_exceptions
             if (std::uncaught_exceptions() == 0)
            [[likely]] f();
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
```
