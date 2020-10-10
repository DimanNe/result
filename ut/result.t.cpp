#include "../src/result_with_using.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>


/// =========================================================================================
/// Helpers

template <class Ok, class Err>
void CheckIsOk(const TResult<Ok, Err> &r) {
   EXPECT_TRUE(r.IsOk());
   EXPECT_FALSE(r.IsErr());
   EXPECT_TRUE(r);
}
template <class Ok, class Err>
void CheckIsErr(const TResult<Ok, Err> &r) {
   EXPECT_FALSE(r.IsOk());
   EXPECT_TRUE(r.IsErr());
   EXPECT_FALSE(r);
}
template <class Ok, class Err, class T>
void CheckOkIs(const TResult<Ok, Err> &r, T &&Val) {
   EXPECT_EQ(r.Ok(), Val);
   EXPECT_EQ(*r, Val);
}
template <class Ok, class Err, class T>
void CheckErrIs(const TResult<Ok, Err> &r, T &&Val) {
   EXPECT_EQ(r.Err(), Val);
}

#define EXPECT_RES_EQ(r, Val) \
   EXPECT_TRUE(r == Val);     \
   EXPECT_TRUE(Val == r);     \
   EXPECT_FALSE(r != Val);    \
   EXPECT_FALSE(Val != r);

#define EXPECT_RES_NOT_EQ(r, Val) \
   EXPECT_TRUE(r != Val);         \
   EXPECT_TRUE(Val != r);         \
   EXPECT_FALSE(r == Val);        \
   EXPECT_FALSE(Val == r);


/// =========================================================================================
/// =========================================================================================
/// Non Void Result

//------------------------------------------------------------------------
// Construction

TEST(TResult, CanBeCreatedViaAssign) {
   { [[maybe_unused]] TResult<int, const char *> r = 2; }
   { [[maybe_unused]] TResult<int, const char *> r = "sdf"; }
}
TEST(TResult, CanNotBeCreatedViaAssignDueToSimilarTypes) {
   // Must not compile
   // [[maybe_unused]] TResult<int, double> r = 2;
}

//------------------------------------------------------------------------
// Copy / Assignment

TEST(TResult, OkResultAssignedOkResult) {
   TResult<int, int> Dst = OkRes(1);
   TResult<int, int> Src = OkRes(41);

   Dst = Src;
   EXPECT_RES_EQ(Dst, Src);
   CheckOkIs(Dst, 41);
}
TEST(TResult, OkResultAssignedErrResult) {
   TResult<int, int> Dst = OkRes(1);
   TResult<int, int> Src = ErrRes(41);

   Dst = Src;
   EXPECT_RES_EQ(Dst, Src);
   CheckErrIs(Dst, 41);
}
TEST(TResult, ErrResultAssignedOkResult) {
   TResult<int, int> Dst = ErrRes(1);
   TResult<int, int> Src = OkRes(41);

   Dst = Src;
   EXPECT_RES_EQ(Dst, Src);
   CheckOkIs(Dst, 41);
}
TEST(TResult, ErrResultAssignedErrResult) {
   TResult<int, int> Dst = ErrRes(1);
   TResult<int, int> Src = ErrRes(41);

   Dst = Src;
   EXPECT_RES_EQ(Dst, Src);
   CheckErrIs(Dst, 41);
}

//------------------------------------------------------------------------
// OkRes() function

TEST(TResult, ResultCreatedWithOkResHoldsOk) {
   TResult<int, int> r = OkRes(1);
   CheckIsOk(r);
}
TEST(TResult, ResultCreatedWithOkResHoldsTheSameValue) {
   TResult<int, int> r = OkRes(1);
   CheckOkIs(r, 1);
}
TEST(TResult, ResultCreatedWithOkResHoldsOk_MovableOnly) {
   TResult<std::unique_ptr<double>, std::unique_ptr<std::string>> r = OkRes(std::make_unique<double>(3.));
   CheckIsOk(r);
}
TEST(TResult, ResultCreatedWithOkResHoldsTheSameValue_MovableOnly) {
   TResult<std::unique_ptr<double>, std::unique_ptr<std::string>> r = OkRes(std::make_unique<double>(3.));
   EXPECT_EQ(**r, 3.);
   EXPECT_EQ(*r.Ok(), 3.);
}
TEST(TResult, OkResWithManyArgsWorksAsEmplace) {
   TResult<std::vector<int>, std::unique_ptr<std::string>> r = OkRes(4, 42);
   ASSERT_TRUE(r);
   const std::vector<int> Expected = {42, 42, 42, 42};
   CheckOkIs(r, Expected);
}

//------------------------------------------------------------------------
// ErrRes() function

TEST(TResult, ResultCreatedWithErrResHoldsErr) {
   TResult<int, int> r = ErrRes(2);
   CheckIsErr(r);
}
TEST(TResult, ResultCreatedWithErrResHoldsTheSameValue) {
   TResult<int, int> r = ErrRes(2);
   CheckErrIs(r, 2);
}
TEST(TResult, ResultCreatedWithErrResHoldsErr_MovableOnly) {
   TResult<std::unique_ptr<double>, std::unique_ptr<std::string>> r =
       ErrRes(std::make_unique<std::string>("asdf"));
   CheckIsErr(r);
}
TEST(TResult, ResultCreatedWithErrResHoldsTheSameValue_MovableOnly) {
   TResult<std::unique_ptr<double>, std::unique_ptr<std::string>> r =
       ErrRes(std::make_unique<std::string>("asdf"));
   EXPECT_EQ(*r.Err(), "asdf");
}

TEST(TResult, ErrResWithManyArgsWorksAsEmplace) {
   TResult<std::vector<int>, std::vector<double>> r = ErrRes(4, 4.2);
   ASSERT_FALSE(r);
   const std::vector<double> Expected = {4.2, 4.2, 4.2, 4.2};
   CheckErrIs(r, Expected);
}

//------------------------------------------------------------------------
// OkResult !=/== OkRes/ErrRes

TEST(TResult, OkResultComparesWithOkRes_WithOneArg) {
   TResult<int, std::string> r = 1;
   EXPECT_RES_NOT_EQ(r, OkRes(2));
   EXPECT_RES_EQ(r, OkRes(1));
   EXPECT_RES_EQ(r, OkRes(1.));

   // Should not compile
   // EXPECT_RES_EQ(r, OkRes(1, 2, 3));
   // auto                      x  = OkRes(2);
   // TResult<int, std::string> r2 = x;
}
TEST(TResult, OkResultComparesWithOkRes_WithZeroArgs) {
   {
      TResult<int, std::string> r = 0;
      // OkRes() effectively corresponds to int(), which is 0, which is equal to r
      EXPECT_RES_EQ(r, OkRes());
   }
   {
      TResult<int, std::string> r = 1;
      EXPECT_RES_NOT_EQ(r, OkRes());
   }
}
TEST(TResult, OkResultComparesWithOkRes_WithManyArgs) {
   TResult<std::string, int> r = "asdf";
   EXPECT_RES_EQ(r, OkRes("asdf", std::string::size_type(4)));
   EXPECT_RES_NOT_EQ(r, OkRes("qwer", std::string::size_type(4)));
   // EXPECT_TRUE(r == OkRes('c', 2, "asdf"));   // Should not compile
}
TEST(TResult, OkResultComparesWithErrRes_WithOneArg) {
   TResult<int, std::string> r = 1;
   EXPECT_RES_NOT_EQ(r, ErrRes("asdf"));
   // EXPECT_RES_NOT_EQ(r, ErrRes(2));  // Should not compile
}
TEST(TResult, OkResultComparesWithErrRes_WithZeroArgs) {
   TResult<std::string, int> r = "asdf";
   EXPECT_RES_NOT_EQ(r, ErrRes());
}
TEST(TResult, OkResultComparesWithErrRes_WithManyArgs) {
   TResult<int, std::string> r = 12;
   EXPECT_RES_NOT_EQ(r, ErrRes("asdf", std::string::size_type(4)));
}

//------------------------------------------------------------------------
// ErrResult !=/== OkRes/ErrRes

TEST(TResult, ErrResultComparesWithErrRes_WithOneArg) {
   TResult<std::string, int> r = 1;
   EXPECT_RES_NOT_EQ(r, ErrRes(2));
   EXPECT_RES_EQ(r, ErrRes(1));
   EXPECT_RES_EQ(r, ErrRes(1.));

   // Should not compile
   // EXPECT_RES_EQ(r, ErrRes(1, 2, 3));
   // auto                      x  = ErrRes(2);
   // TResult<int, std::string> r2 = x;
}
TEST(TResult, ErrResultComparesWithErrRes_WithZeroArgs) {
   {
      TResult<std::string, int> r = 0;
      // ErrRes() effectively corresponds to int(), which is 0, which is equal to r
      EXPECT_RES_EQ(r, ErrRes());
   }
   {
      TResult<std::string, int> r = 1;
      EXPECT_RES_NOT_EQ(r, ErrRes());
   }
}
TEST(TResult, ErrResultComparesWithErrRes_WithManyArgs) {
   TResult<int, std::string> r = "asdf";
   EXPECT_RES_EQ(r, ErrRes("asdf", std::string::size_type(4)));
   EXPECT_RES_NOT_EQ(r, ErrRes("qwer", std::string::size_type(4)));
   // EXPECT_TRUE(r == ErrRes('c', 2, "asdf"));   // Should not compile
}
TEST(TResult, ErrResultComparesWithOkRes_WithOneArg) {
   TResult<std::string, int> r = 1;
   EXPECT_RES_NOT_EQ(r, OkRes("asdf"));
   // EXPECT_RES_NOT_EQ(r, ErrRes(2));  // Should not compile
}
TEST(TResult, ErrResultComparesWithOkRes_WithZeroArgs) {
   TResult<int, std::string> r = "asdf";
   EXPECT_RES_NOT_EQ(r, OkRes());
}
TEST(TResult, ErrResultComparesWithOkRes_WithManyArgs) {
   TResult<std::string, int> r = 12;
   EXPECT_RES_NOT_EQ(r, OkRes("asdf", std::string::size_type(4)));
}

//------------------------------------------------------------------------
// Result !=/== Result

TEST(TResult, OkResultComparesWithOkResult) {
   TResult<std::string, int> r1 = "asdf";
   TResult<std::string, int> r2 = "asdf";
   TResult<std::string, int> r3 = "qwer";
   EXPECT_RES_EQ(r1, r2);
   EXPECT_RES_NOT_EQ(r1, r3);
}
TEST(TResult, OkResultComparesWithErrResult) {
   TResult<std::string, int> r1 = 12;
   TResult<std::string, int> r2 = 12;
   TResult<std::string, int> r3 = 1;
   EXPECT_RES_EQ(r1, r2);
   EXPECT_RES_NOT_EQ(r1, r3);
}

/// =========================================================================================
/// =========================================================================================
/// Void Result

//------------------------------------------------------------------------
// Construction

TEST(TResultVoid, CanBeCreatedViaAssign) {
   [[maybe_unused]] TResult<void, const char *> r = "sdf";
}

//------------------------------------------------------------------------
// Copy / Assignment

TEST(TResultVoid, OkResultAssignedOkResult) {
   TResult<void, int> Dst;
   TResult<void, int> Src;

   Dst = Src;
   EXPECT_RES_EQ(Dst, Src);
   EXPECT_TRUE(Dst.IsOk());
}
TEST(TResultVoid, OkResultAssignedErrResult) {
   TResult<void, int> Dst;
   TResult<void, int> Src = ErrRes(41);

   Dst = Src;
   EXPECT_RES_EQ(Dst, Src);
   CheckErrIs(Dst, 41);
}
TEST(TResultVoid, ErrResultAssignedOkResult) {
   TResult<void, int> Dst = ErrRes(1);
   TResult<void, int> Src;

   Dst = Src;
   EXPECT_RES_EQ(Dst, Src);
   EXPECT_TRUE(Dst.IsOk());
}
TEST(TResultVoid, ErrResultAssignedErrResult) {
   TResult<void, int> Dst = ErrRes(1);
   TResult<void, int> Src = ErrRes(41);

   Dst = Src;
   EXPECT_RES_EQ(Dst, Src);
   CheckErrIs(Dst, 41);
}

//------------------------------------------------------------------------
// OkRes() function

TEST(TResultVoid, ResultCreatedWithOkResHoldsOk) {
   TResult<void, int> r = OkRes();
   CheckIsOk(r);
}
TEST(TResultVoid, CannotBeCreateWithOkResWithAnArg) {
   // Must not compile
   // [[maybe_unused]] TResult<void, int> r = OkRes(1);
   // [[maybe_unused]] TResult<void, int> r = OkRes(1, 2, 3);
}

//------------------------------------------------------------------------
// ErrRes() function

TEST(TResultVoid, ResultCreatedWithErrResHoldsErr) {
   TResult<void, int> r = ErrRes(2);
   CheckIsErr(r);
}
TEST(TResultVoid, ResultCreatedWithErrResHoldsTheSameValue) {
   TResult<void, int> r = ErrRes(2);
   CheckErrIs(r, 2);
}
TEST(TResultVoid, ResultCreatedWithErrResHoldsErr_MovableOnly) {
   TResult<void, std::unique_ptr<std::string>> r = ErrRes(std::make_unique<std::string>("asdf"));
   CheckIsErr(r);
}
TEST(TResultVoid, ResultCreatedWithErrResHoldsTheSameValue_MovableOnly) {
   TResult<void, std::unique_ptr<std::string>> r = ErrRes(std::make_unique<std::string>("asdf"));
   EXPECT_EQ(*r.Err(), "asdf");
}
TEST(TResultVoid, ErrResWithManyArgsWorksAsEmplace) {
   TResult<void, std::vector<double>> r = ErrRes(4, 4.2);
   ASSERT_FALSE(r);
   const std::vector<double> Expected = {4.2, 4.2, 4.2, 4.2};
   CheckErrIs(r, Expected);
}

//------------------------------------------------------------------------
// OkResult !=/== OkRes/ErrRes

TEST(TResultVoid, OkResultComparesWithOkRes_WithOneArg) {
   // Should not compile
   // TResult<void, int> r;
   // EXPECT_RES_NOT_EQ(r, OkRes(0));
}
TEST(TResultVoid, OkResultComparesWithOkRes_WithZeroArgs) {
   TResult<void, int> r;
   EXPECT_RES_EQ(r, OkRes());
}
TEST(TResultVoid, OkResultComparesWithOkRes_WithManyArgs) {
   // Should not compile
   // TResult<void, int> r;
   // EXPECT_RES_NOT_EQ(r, OkRes("asdf", std::string::size_type(4)));
}
TEST(TResultVoid, OkResultComparesWithErrRes_WithOneArg) {
   TResult<void, int> r;
   EXPECT_RES_NOT_EQ(r, ErrRes(0));
   // EXPECT_RES_NOT_EQ(r, ErrRes(2));  // Should not compile
}
TEST(TResultVoid, OkResultComparesWithErrRes_WithZeroArgs) {
   TResult<void, int> r;
   EXPECT_RES_NOT_EQ(r, ErrRes());
}
TEST(TResultVoid, OkResultComparesWithErrRes_WithManyArgs) {
   TResult<void, std::string> r;
   EXPECT_RES_NOT_EQ(r, ErrRes("asdf", std::string::size_type(4)));
}

//------------------------------------------------------------------------
// ErrResult !=/== OkRes/ErrRes

TEST(TResultVoid, ErrResultComparesWithErrRes_WithOneArg) {
   TResult<void, int> r = 1;
   EXPECT_RES_NOT_EQ(r, ErrRes(2));
   EXPECT_RES_EQ(r, ErrRes(1));
   EXPECT_RES_EQ(r, ErrRes(1.));
}
TEST(TResultVoid, ErrResultComparesWithErrRes_WithZeroArgs) {
   {
      TResult<void, int> r = 0;
      // ErrRes() effectively corresponds to int(), which is 0, which is equal to r
      EXPECT_RES_EQ(r, ErrRes());
   }
   {
      TResult<void, int> r = 1;
      EXPECT_RES_NOT_EQ(r, ErrRes());
   }
}
TEST(TResultVoid, ErrResultComparesWithErrRes_WithManyArgs) {
   TResult<void, std::string> r = "asdf";
   EXPECT_RES_EQ(r, ErrRes("asdf", std::string::size_type(4)));
   EXPECT_RES_NOT_EQ(r, ErrRes("qwer", std::string::size_type(4)));
}
TEST(TResultVoid, ErrResultComparesWithOkRes_WithOneArg) {
   // Should not compile
   // TResult<void, int> r = 1;
   // EXPECT_RES_NOT_EQ(r, OkRes("asdf"));
}
TEST(TResultVoid, ErrResultComparesWithOkRes_WithZeroArgs) {
   TResult<void, int> r = 1;
   EXPECT_RES_NOT_EQ(r, OkRes());
}
TEST(TResultVoid, ErrResultComparesWithOkRes_WithManyArgs) {
   // Should not compile
   // TResult<void, std::string> r = "asdf";
   // EXPECT_RES_NOT_EQ(r, OkRes("asdf", std::string::size_type(4)));
}


//------------------------------------------------------------------------
// Result !=/== Result

TEST(TResultVoid, OkResultComparesWithOkResult) {
   TResult<void, int> r1;
   TResult<void, int> r2;
   EXPECT_RES_EQ(r1, r2);
}
TEST(TResultVoid, OkResultComparesWithErrResult) {
   TResult<void, int> r1;
   TResult<void, int> r3 = 1;
   EXPECT_RES_NOT_EQ(r1, r3);
}
