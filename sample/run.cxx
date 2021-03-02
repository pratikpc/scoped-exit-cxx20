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

int main()
{
   no_throw();
   std::cout << "\n\n\n";
   // GSL would not have been able to handle it
   throw_but_catch();
   std::cout << "\n\n\n";
   dont_check_for_uncaught_exceptions();
   std::cout << "\n\n\n";
   throw_outside_cleanup_not_called();
}