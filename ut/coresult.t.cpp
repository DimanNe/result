#include "../src/coresult_with_using.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <util/compiler.h>
#include <util/memory.h>

using ::testing::EndsWith;
using ::testing::StartsWith;

// Help:
// https://blog.panicsoftware.com/your-first-coroutine/
// https://blog.panicsoftware.com/co_awaiting-coroutines/

// ======================================================================================================
// ======================================================================================================
// NonVoid Result Tests

struct TCoResult_NonVoid_SameTypes: public ::testing::Test {
   TCoResult<int, double> F1() {
      static int Counter = 1;
      if(++Counter % 2 == 0)
         return ErrRes(2);
      return OkRes(2);
   }
   TCoResult<int, double> F2() {
      const int Ok = co_await F1().OrReturn(1);
      co_return OkRes(Ok * 2);
   }
};
TEST_F(TCoResult_NonVoid_SameTypes, Test) {
   {
      TCoResult<int, double> Result = F2();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_EQ(Result.Err(), 1);
   }
   {
      TCoResult<int, double> Result = F2();
      EXPECT_TRUE(Result.IsOk());
      EXPECT_EQ(Result.Ok(), 4);
   }
}

// ------------------------------------------------------------------------------------------------------

struct TCoResult_NonVoid_BothAreMovableOnly: public ::testing::Test {
   TCoResult<std::unique_ptr<int>, std::unique_ptr<double>> F1() {
      static int Counter = 1;
      if(++Counter % 2 == 0)
         return ErrRes(MakeUnique(10.));
      return OkRes(MakeUnique(2));
   }
   TCoResult<std::unique_ptr<int>, std::unique_ptr<double>> F2() {
      std::unique_ptr<int> Ok = std::move(co_await F1().OrReturn(MakeUnique(5.)));
      co_return Ok;
   }
};
TEST_F(TCoResult_NonVoid_BothAreMovableOnly, Test) {
   {
      TCoResult<std::unique_ptr<int>, std::unique_ptr<double>> Result = F2();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_EQ(*Result.Err(), 5);
   }
   {
      TCoResult<std::unique_ptr<int>, std::unique_ptr<double>> Result = F2();
      EXPECT_TRUE(Result.IsOk());
      EXPECT_EQ(*Result.Ok(), 2);
   }
}

// ------------------------------------------------------------------------------------------------------

struct TCoResult_NonVoid_DifferentTypes: public ::testing::Test {
   TCoResult<std::string, double> F1() {
      static int Counter = 1;
      if(++Counter % 2 == 0)
         return 10.;
      return "Ok";
   }
   struct TSomeStruct {
      bool operator==(const TSomeStruct &) const {
         return true;
      }
   };
   TCoResult<std::unique_ptr<int>, TSomeStruct> F2() {
      std::string Ok = co_await F1().OrReturn(TSomeStruct());
      co_return MakeUnique(Ok.size());
   }
};
TEST_F(TCoResult_NonVoid_DifferentTypes, Test) {
   {
      TCoResult<std::unique_ptr<int>, TSomeStruct> Result = F2();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_EQ(Result.Err(), TSomeStruct());
   }
   {
      TCoResult<std::unique_ptr<int>, TSomeStruct> Result = F2();
      EXPECT_TRUE(Result.IsOk());
      EXPECT_EQ(*Result.Ok(), 2);
   }
}

// ------------------------------------------------------------------------------------------------------

struct TCoResult_NonVoid_OrReturns: public ::testing::Test {
   TCoResult<double, std::string> F1() {
      static int Counter = 1;
      if(++Counter % 2 == 0)
         return "Err";
      return 10.;
   }
   TCoResult<double, std::string> OrPrependErrMsgAndReturn() {
      double Ok = co_await F1().OrPrependErrMsgAndReturn("qwe");
      co_return Ok;
   }
   TCoResult<double, std::string> OrReturnNewErr() {
      double Ok = co_await F1().OrReturnNewErr([](std::string &&ExistingErr) {
         EXPECT_EQ(ExistingErr, "Err");
         return "New Errorrrr";
      });
      co_return Ok;
   }
   TCoResult<double, std::string> OrReturn() {
      double Ok = co_await F1().OrReturn("x");
      co_return Ok;
   }
};
TEST_F(TCoResult_NonVoid_OrReturns, OrPrependErrMsgAndReturn) {
   {
      TCoResult<double, std::string> Result = OrPrependErrMsgAndReturn();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_THAT(Result.Err(), StartsWith("qwe"));
      EXPECT_THAT(Result.Err(), EndsWith("Err"));
   }
   {
      TCoResult<double, std::string> Result = OrPrependErrMsgAndReturn();
      EXPECT_TRUE(Result.IsOk());
      EXPECT_EQ(Result.Ok(), 10);
   }
}
TEST_F(TCoResult_NonVoid_OrReturns, OrReturnNewErr) {
   {
      TCoResult<double, std::string> Result = OrReturnNewErr();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_THAT(Result.Err(), "New Errorrrr");
   }
   {
      TCoResult<double, std::string> Result = OrReturnNewErr();
      EXPECT_TRUE(Result.IsOk());
      EXPECT_EQ(Result.Ok(), 10);
   }
}
TEST_F(TCoResult_NonVoid_OrReturns, OrReturn) {
   {
      TCoResult<double, std::string> Result = OrReturn();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_THAT(Result.Err(), "x");
   }
   {
      TCoResult<double, std::string> Result = OrReturn();
      EXPECT_TRUE(Result.IsOk());
      EXPECT_EQ(Result.Ok(), 10);
   }
}

// ------------------------------------------------------------------------------------------------------

struct TCoResult_NonVoid_OrReturns_WithMovableOnly: public ::testing::Test {
   TCoResult<double, std::unique_ptr<int>> F1() {
      static int Counter = 1;
      if(++Counter % 2 == 0)
         return MakeUnique(1);
      return 10.;
   }
   TCoResult<double, std::unique_ptr<int>> OrReturnNewErr() {
      double Ok = co_await F1().OrReturnNewErr([](std::unique_ptr<int> &&ExistingErr) {
         EXPECT_EQ(*ExistingErr, 1);
         return MakeUnique(5);
      });
      co_return Ok;
   }
   TCoResult<double, std::unique_ptr<int>> OrReturn() {
      double Ok = co_await F1().OrReturn(MakeUnique(5));
      co_return Ok;
   }
};
TEST_F(TCoResult_NonVoid_OrReturns_WithMovableOnly, OrReturnNewErr) {
   {
      TCoResult<double, std::unique_ptr<int>> Result = OrReturnNewErr();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_THAT(*Result.Err(), 5);
   }
   {
      TCoResult<double, std::unique_ptr<int>> Result = OrReturnNewErr();
      EXPECT_TRUE(Result.IsOk());
      EXPECT_EQ(Result.Ok(), 10);
   }
}
TEST_F(TCoResult_NonVoid_OrReturns_WithMovableOnly, OrReturn) {
   {
      TCoResult<double, std::unique_ptr<int>> Result = OrReturn();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_THAT(*Result.Err(), 5);
   }
   {
      TCoResult<double, std::unique_ptr<int>> Result = OrReturn();
      EXPECT_TRUE(Result.IsOk());
      EXPECT_EQ(Result.Ok(), 10);
   }
}

// ------------------------------------------------------------------------------------------------------

struct TCoResult_NonVoid_OrReturns_WithReferences: public ::testing::Test {
   TCoResult<int, std::string> F1() {
      static int Counter = 1;
      if(++Counter % 2 == 0)
         return "Err";
      return 10;
   }
   std::string                 ErrReference = "ErrReference";
   TCoResult<int, std::string> OrReturnNewErr() {
      int Ok = co_await F1().OrReturnNewErr([this](std::string &&ExistingErr) -> std::string & {
         EXPECT_EQ(ExistingErr, "Err");
         return ErrReference;
      });
      co_return Ok;
   }
   TCoResult<int, std::string> OrReturn() {
      int Ok = co_await F1().OrReturn(ErrReference);
      co_return Ok;
   }
};
TEST_F(TCoResult_NonVoid_OrReturns_WithReferences, OrReturnNewErr) {
   {
      TCoResult<int, std::string> Result = OrReturnNewErr();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_THAT(Result.Err(), "ErrReference");
      Result.Err().clear();
      EXPECT_THAT(ErrReference, "ErrReference");
   }
   {
      TCoResult<int, std::string> Result = OrReturnNewErr();
      EXPECT_TRUE(Result.IsOk());
      EXPECT_EQ(Result.Ok(), 10);
   }
}
TEST_F(TCoResult_NonVoid_OrReturns_WithReferences, OrReturn) {
   {
      TCoResult<int, std::string> Result = OrReturn();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_THAT(Result.Err(), "ErrReference");
      Result.Err().clear();
      EXPECT_THAT(ErrReference, "ErrReference");
   }
   {
      TCoResult<int, std::string> Result = OrReturn();
      EXPECT_TRUE(Result.IsOk());
      EXPECT_EQ(Result.Ok(), 10);
   }
}

// ======================================================================================================
// ======================================================================================================
// Void Result Tests


struct TCoResult_Void_SameTypes: public ::testing::Test {
   TCoResult<void, double> F1() {
      static int Counter = 1;
      if(++Counter % 2 == 0)
         return ErrRes(2);
      return {};
   }
   TCoResult<void, double> F2() {
      co_await F1().OrReturn(1);
   }
};
TEST_F(TCoResult_Void_SameTypes, Test) {
   {
      TCoResult<void, double> Result = F2();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_EQ(Result.Err(), 1);
   }
   {
      TCoResult<void, double> Result = F2();
      EXPECT_TRUE(Result.IsOk());
   }
}

// ------------------------------------------------------------------------------------------------------

struct TCoResult_Void_MovableOnly: public ::testing::Test {
   TCoResult<void, std::unique_ptr<double>> F1() {
      static int Counter = 1;
      if(++Counter % 2 == 0)
         return ErrRes(MakeUnique(10.));
      return {};
   }
   TCoResult<void, std::unique_ptr<double>> F2() {
      co_await F1().OrReturn(MakeUnique(5.));
      co_return;
   }
};
TEST_F(TCoResult_Void_MovableOnly, Test) {
   {
      TCoResult<void, std::unique_ptr<double>> Result = F2();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_EQ(*Result.Err(), 5);
   }
   {
      TCoResult<void, std::unique_ptr<double>> Result = F2();
      EXPECT_TRUE(Result.IsOk());
   }
}

// ------------------------------------------------------------------------------------------------------

struct TCoResult_Void_DifferentTypes: public ::testing::Test {
   TCoResult<void, double> F1() {
      static int Counter = 1;
      if(++Counter % 2 == 0)
         return 10.;
      return {};
   }
   struct TSomeStruct {
      bool operator==(const TSomeStruct &) const {
         return true;
      }
   };
   TCoResult<void, TSomeStruct> F2() {
      co_await F1().OrReturn(TSomeStruct());
   }
};
TEST_F(TCoResult_Void_DifferentTypes, Test) {
   {
      TCoResult<void, TSomeStruct> Result = F2();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_EQ(Result.Err(), TSomeStruct());
   }
   {
      TCoResult<void, TSomeStruct> Result = F2();
      EXPECT_TRUE(Result.IsOk());
   }
}

// ------------------------------------------------------------------------------------------------------

struct TCoResult_Void_OrReturns: public ::testing::Test {
   TCoResult<void, std::string> F1() {
      static int Counter = 1;
      if(++Counter % 2 == 0)
         return "Err";
      return {};
   }
   TCoResult<void, std::string> OrPrependErrMsgAndReturn() {
      co_await F1().OrPrependErrMsgAndReturn("qwe");
   }
   TCoResult<void, std::string> OrReturnNewErr() {
      co_await F1().OrReturnNewErr([](std::string &&ExistingErr) {
         EXPECT_EQ(ExistingErr, "Err");
         return "New Errorrrr";
      });
   }
   TCoResult<void, std::string> OrReturn() {
      co_await F1().OrReturn("x");
   }
};
TEST_F(TCoResult_Void_OrReturns, OrPrependErrMsgAndReturn) {
   {
      TCoResult<void, std::string> Result = OrPrependErrMsgAndReturn();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_THAT(Result.Err(), StartsWith("qwe"));
      EXPECT_THAT(Result.Err(), EndsWith("Err"));
   }
   {
      TCoResult<void, std::string> Result = OrPrependErrMsgAndReturn();
      EXPECT_TRUE(Result.IsOk());
   }
}
TEST_F(TCoResult_Void_OrReturns, OrReturnNewErr) {
   {
      TCoResult<void, std::string> Result = OrReturnNewErr();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_THAT(Result.Err(), "New Errorrrr");
   }
   {
      TCoResult<void, std::string> Result = OrReturnNewErr();
      EXPECT_TRUE(Result.IsOk());
   }
}
TEST_F(TCoResult_Void_OrReturns, OrReturn) {
   {
      TCoResult<void, std::string> Result = OrReturn();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_THAT(Result.Err(), "x");
   }
   {
      TCoResult<void, std::string> Result = OrReturn();
      EXPECT_TRUE(Result.IsOk());
   }
}

// ------------------------------------------------------------------------------------------------------

struct TCoResult_Void_OrReturns_WithMovableOnly: public ::testing::Test {
   TCoResult<void, std::unique_ptr<int>> F1() {
      static int Counter = 1;
      if(++Counter % 2 == 0)
         return MakeUnique(1);
      return {};
   }
   TCoResult<void, std::unique_ptr<int>> OrReturnNewErr() {
      co_await F1().OrReturnNewErr([](std::unique_ptr<int> &&ExistingErr) {
         EXPECT_EQ(*ExistingErr, 1);
         return MakeUnique(5);
      });
   }
   TCoResult<void, std::unique_ptr<int>> OrReturn() {
      co_await F1().OrReturn(MakeUnique(5));
   }
};
TEST_F(TCoResult_Void_OrReturns_WithMovableOnly, OrReturnNewErr) {
   {
      TCoResult<void, std::unique_ptr<int>> Result = OrReturnNewErr();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_THAT(*Result.Err(), 5);
   }
   {
      TCoResult<void, std::unique_ptr<int>> Result = OrReturnNewErr();
      EXPECT_TRUE(Result.IsOk());
   }
}
TEST_F(TCoResult_Void_OrReturns_WithMovableOnly, OrReturn) {
   {
      TCoResult<void, std::unique_ptr<int>> Result = OrReturn();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_THAT(*Result.Err(), 5);
   }
   {
      TCoResult<void, std::unique_ptr<int>> Result = OrReturn();
      EXPECT_TRUE(Result.IsOk());
   }
}

// ------------------------------------------------------------------------------------------------------

struct TCoResult_Void_OrReturns_WithReferences: public ::testing::Test {
   TCoResult<void, std::string> F1() {
      static int Counter = 1;
      if(++Counter % 2 == 0)
         return "Err";
      return {};
   }
   std::string                  ErrReference = "ErrReference";
   TCoResult<void, std::string> OrReturnNewErr() {
      co_await F1().OrReturnNewErr([this](std::string &&ExistingErr) -> std::string & {
         EXPECT_EQ(ExistingErr, "Err");
         return ErrReference;
      });
   }
   TCoResult<void, std::string> OrReturn() {
      co_await F1().OrReturn(ErrReference);
   }
};
TEST_F(TCoResult_Void_OrReturns_WithReferences, OrReturnNewErr) {
   {
      TCoResult<void, std::string> Result = OrReturnNewErr();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_THAT(Result.Err(), "ErrReference");
      Result.Err().clear();
      EXPECT_THAT(ErrReference, "ErrReference");
   }
   {
      TCoResult<void, std::string> Result = OrReturnNewErr();
      EXPECT_TRUE(Result.IsOk());
   }
}
TEST_F(TCoResult_Void_OrReturns_WithReferences, OrReturn) {
   {
      TCoResult<void, std::string> Result = OrReturn();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_THAT(Result.Err(), "ErrReference");
      Result.Err().clear();
      EXPECT_THAT(ErrReference, "ErrReference");
   }
   {
      TCoResult<void, std::string> Result = OrReturn();
      EXPECT_TRUE(Result.IsOk());
   }
}

// ======================================================================================================
// ======================================================================================================
// Mixed

struct TCoResult_Mixed_NonVoidToVoid: public ::testing::Test {
   TCoResult<int, std::string> F1() {
      static int Counter = 1;
      if(++Counter % 2 == 0)
         return "Err";
      return 1;
   }
   TCoResult<void, double> F2() {
      co_await F1().OrReturn(2);
   }
};
TEST_F(TCoResult_Mixed_NonVoidToVoid, OrReturnNewErr) {
   {
      TCoResult<void, double> Result = F2();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_THAT(Result.Err(), 2);
   }
   {
      TCoResult<void, double> Result = F2();
      EXPECT_TRUE(Result.IsOk());
   }
}


struct TCoResult_Mixed_VoidNonVoid: public ::testing::Test {
   TCoResult<void, std::string> F1() {
      static int Counter = 1;
      if(++Counter % 2 == 0)
         return "Err";
      return {};
   }
   TCoResult<std::string, double> F2() {
      co_await F1().OrReturn(2);
      co_return "Ok";
   }
};
TEST_F(TCoResult_Mixed_VoidNonVoid, OrReturnNewErr) {
   {
      TCoResult<std::string, double> Result = F2();
      EXPECT_TRUE(Result.IsErr());
      EXPECT_THAT(Result.Err(), 2);
   }
   {
      TCoResult<std::string, double> Result = F2();
      EXPECT_TRUE(Result.IsOk());
      EXPECT_THAT(Result.Ok(), "Ok");
   }
}
