#include "../src/coresult.h"

#include <experimental/coroutine>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <util/compiler.h>
#include <variant>

// Help:
// https://blog.panicsoftware.com/your-first-coroutine/
// https://blog.panicsoftware.com/co_awaiting-coroutines/

using namespace NDiRes;

class TRes {
public:
   TRes() = default;
   TRes(int v): Data(std::in_place_index<0>, v) {}
   TRes(std::string s): Data(std::in_place_index<1>, s) {}
   TRes(const TRes &Another) {
      *this = Another;
   }
   TRes &operator=(const TRes &Another) {
      Data = Another.Data;
      return *this;
   }

   int Ok() const {
      return std::get<0>(Data);
   }
   std::string Err() const {
      return std::get<1>(Data);
   }
   bool IsOk() const {
      return Data.index() == 0;
   }
   bool IsErr() const {
      return !IsOk();
   }

private:
   std::variant<int, std::string> Data;
};
std::ostream &operator<<(std::ostream &Out, const TRes &Res) {
   if(Res.IsOk())
      Out << "Ok(" << Res.Ok() << ")";
   else
      Out << "Err(" << Res.Err() << ")";
   return Out;
}


class TCoRes: public TRes {
   class TPromise;

public:
   using promise_type = TPromise;

public:
   using THandle      = std::experimental::coroutine_handle<TPromise>;

   using TRes::TRes;

private:
   struct TReturn {
      TReturn(const TReturn &) = delete;
      TReturn &operator=(const TReturn &) = delete;
      TReturn(THandle handle) {
         std::cout << "TReturn: TReturn(THandle handle): this: " << static_cast<void *>(this) << std::endl;
         assert(handle);
         Handle = handle;
      }
      ~TReturn() {
         Handle.destroy();
      }
      THandle Handle;
   };

public:
   TCoRes(const TReturn &ReturnObj) {
      std::cout << "TCoRes: TCoRes(TReturn) from: " << static_cast<const void *>(&ReturnObj) << " "
                << ReturnObj.Handle.promise().RetRes << std::endl;
      static_cast<TRes &>(*this) = ReturnObj.Handle.promise().RetRes;
   }
   ~TCoRes() {
      std::cout << "TCoRes: Dtor" << std::endl;
   }

   // ===================================
private:
   class TPromise {
   public:
      using THandle = std::experimental::coroutine_handle<TPromise>;
      TCoRes::TReturn get_return_object() {
         std::cout << "TPromise: get_return_object" << std::endl;
         return {THandle::from_promise(*this)};
      }
      TPromise() {
         std::cout << "TPromise: Default Ctor" << std::endl;
      }
      ~TPromise() {
         std::cout << "TPromise: Dtor" << std::endl;
      }
      std::experimental::suspend_never initial_suspend() {
         std::cout << "TPromise: initial_suspend" << std::endl;
         return {};
      }
      std::experimental::suspend_always final_suspend() {
         // Postpone coroutine destructions (in particular TPromise),
         // so that TReturn still hold a valid reference to coroutine handle
         // which holds a reference to this Promise, which is used in ctor of TCoRes
         // to obtain RetRes.
         std::cout << "TPromise: final_suspend" << std::endl;
         return {};
      }
      TRes RetRes;   // Change it to pointer as well?
      void return_value(TRes Res) {
         std::cout << "TPromise: return_value: Remembering: " << Res << std::endl;
         RetRes = Res;
      }
      void return_value(double Res) {
         std::cout << "TPromise: return_value: Remembering: " << Res << std::endl;
         RetRes = Res;
      }
      template <class T>
      void return_value(T Res) {
         std::cout << "TPromise: return_value: Remembering: " << Res << std::endl;
         RetRes = Res;
      }
      void unhandled_exception() {
         std::cout << "TPromise: unhandled_exception" << std::endl;
         std::terminate();
      }
   };

private:
   struct TAwaitabler {   // Awaiter + Awaitable
      TRes Result;   // Change to pointer to this result (instead of making copy/moving?)

      bool await_ready() {   // Need to decide what to do: continue this function or return to caller
         const bool Ready = Result.IsOk();
         std::cout << "TAwaiter::await_ready(): Ready = " << Ready << std::endl;
         return Ready;
      }
      void await_suspend(THandle Handle) {
         // Result is Err or final_suspend returned suspend_always => Return control-flow to the caller
         std::cout << "TAwaiter::await_suspend()" << std::endl;
         Handle.promise().return_value(Result);
      }
      int await_resume() {   // Result is Ok => Return its Ok() and continue execution
         std::cout << "TAwaiter::await_resume(). Data = " << Result << std::endl;
         return Result.Ok();
      }
   };

public:
   TAwaitabler OrNestAndReturn(std::string Prefix) {
      if(IsOk()) {
         std::cout << "TCoRes::OrNestAndReturn(): IsOk() == true, Returning Ok TAwaitable" << std::endl;
         return {Ok()};
      } else {
         std::cout << "TCoRes::OrNestAndReturn(): IsOk() == false, Returning Err TAwaitable" << std::endl;
         return {Prefix + Err()};
      }
   };
};

TCoRes ConnectTo() {
   // TCoRes Result("Failed to create socket");
   TCoRes Result(1234);
   int Socket = co_await Result.OrNestAndReturn("Failed to connect to 1.2.3.4: ");
   co_return Socket;
   // co_return 1234;
}


TEST(Util_CoResult_Main, Main) {
   TCoRes Res = ConnectTo();
   std::cout << "Res: " << Res << std::endl;
}

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


TEST(Util_CoResult_Main_Real, Main) {
   std::cout << ReadSettings() << std::endl;
}
