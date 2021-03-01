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
8. At the same time, we need to ensure that, genuine users who need noexcept support should also receive it?
9. This is easily solved with [std::is_nothrow_invocable](https://en.cppreference.com/w/cpp/types/is_invocable)   
   Microsoft's GSL is not using this because this was introduced in C++17

---

## [Usage](sample/run.cxx)

### No Throw

```cpp
#include <utility>
#include <scoped_exit.hxx>
#include <iostream>

void no_throw ()
{
   // Note that as the Function provided here is noexcept
   // So will our destructor
   // Note that throwing in a nothrow lambda
   // Will lead to noexcept semantics
   pc::scoped_exit nothrow {[]() noexcept{
      std::cout << "Perform cleanup here\n";
   }};
   std::cout << "Function Started\n";
   std::cout << "Function Ended\n";
}
```

### Throw

```cpp
#include <utility>
#include <scoped_exit.hxx>
#include <iostream>

void throw_but_catch ()
{
    try{
        pc::scoped_exit throws {[]() {
            std::cout << "During Cleanup, we had to throw\n";
            throw std::exception("Sorry. Something during Cleanup errored out");
        }};

        std::cout << "Block Started\n";
        std::cout << "Block Ended\n";
        // Destructor called here
    }
    catch (std::exception ex)
    {
        std::cout << "Exception caught : " << ex.what() << "\n";
    }
}
```

---

## [Our code](header/scoped_exit.hxx)

```cpp
namespace pc
{
   template<typename F>
   // Change Requires to an ugly enable_if_t or a static_assert for C++17 support
   requires(std::is_invocable_v<F>)
   struct scoped_exit
   {
      // Store the function only
      // No need to store a 2nd variable
      F f;
      scoped_exit(F&&f): f(std::forward<F>(f)){}

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
      scoped_exit(scoped_exit&&) = delete;
      scoped_exit& operator=(scoped_exit&&) = delete;
      scoped_exit& operator=(const scoped_exit&) = delete;
   };

   // Ensure naming convention matches GSL
   // To make switching from GSL to pc::scoped_exit easy
   template<typename F>
   using finally = scoped_exit<F>;
   template<typename F>
   using final_action = scoped_exit<F>;
}
```
