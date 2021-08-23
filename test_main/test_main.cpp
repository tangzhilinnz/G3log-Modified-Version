/** ==========================================================================
* 2011 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
* with no warranties. This code is yours to share, use and modify with no
* strings attached and no restrictions or obligations.
* ============================================================================*/

#include <gtest/gtest.h>
#include <iostream>

int main(int argc, char *argv[])
{
   testing::InitGoogleTest(&argc, argv);
   // After defining your tests, you can run them with RUN_ALL_TESTS(), which 
   // returns 0 if all the tests are successful, or 1 otherwise. Note that 
   // RUN_ALL_TESTS() runs all tests in your link unit¨Cthey can be from 
   // different test suites, or even different source files.
   int return_value = RUN_ALL_TESTS();
   std::cout << "FINISHED WITH THE TESTING" << std::endl;
   return return_value;
}

