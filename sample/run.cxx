#include <scoped_exit.hxx>

#include <iostream>

void no_throw()
{
   // Note that as the Function provided here is noexcept
   // So will our destructor

   // Note that throwing in a nothrow lambda
   // Will lead to noexcept semantics
   pc::scoped_exit nothrow{[]() noexcept { std::cout << "Perform cleanup here\n"; }};
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

      pc::scoped_exit throws{[]() {
         std::cout << "During Cleanup, we had to throw\n";
         throw std::runtime_error("Sorry. Something during Cleanup errored out");
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
int main()
{
   no_throw();
   // GSL would not have been able to handle it
   throw_but_catch();
}