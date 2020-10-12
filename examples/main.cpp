#include "../src/coresult_with_using.h"

#include <iostream>

TCoResult<int, std::string> CreateSocket() {
   static int Counter = 1;
   if(++Counter % 2 == 0)
      co_return "Failed to CreateSocket: SysErr: EINVAL Invalid argument";
   co_return 42;
}
TCoResult<int, std::string> OpenSocket() {
   const int Socket = 2 + co_await CreateSocket().OrNestAndReturn();
   co_return Socket * 2;
}
TCoResult<int, std::string> ConnectSocket() {
   co_return co_await OpenSocket().OrNestAndReturn();
}
TCoResult<std::string, std::string> ReadSettings() {
   co_await ConnectSocket().OrNestAndReturn();
   co_return OkRes("Here is our settings");
}


int main(int, char *[]) {
   std::cout << ReadSettings() << std::endl << std::endl;
   std::cout << "==== Second attempt ====" << std::endl;
   std::cout << ReadSettings() << std::endl << std::endl;
   return 0;

   // https://godbolt.org/z/8hxKTW

   // Expected output:
   // Err(Failed to ReadSettings @ Line:20: Failed to ConnectSocket @ Line:17: Failed to OpenSocket @ Line:13:
   // Failed to CreateSocket: SysErr: EINVAL Invalid argument)
   //
   // ==== Second attempt ====
   // Ok(Here is our settings)
}
