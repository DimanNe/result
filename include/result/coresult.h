#pragma once

#include "result.h"

#include <cassert>
#include <coroutine>

namespace NDiRes {

   template <class TString, class T>
   concept StringPrependable = requires(TString String, T a) {
      String + a;
   };

   template <class TFrom, class TTo>
   concept ConvertibleTo =
       std::is_convertible_v<TFrom, TTo> &&requires(std::add_rvalue_reference_t<TFrom> (&f)()) {
      static_cast<TTo>(f());
   };

   namespace NPrivate {
      template <class T, class U>
      concept NotSame = std::is_same_v<T, U> == false;
   }
   template <class T, class U>
   concept NotSameAs = NPrivate::NotSame<T, U> &&NPrivate::NotSame<U, T>;

   template <class TCallable, class TOldErr>
   concept CallableCanGenerateNewErrFromOldErr = requires(TCallable Callable, TOldErr Err) {
      // clang-format off
      { Callable(std::move(Err)) } -> NotSameAs<void>;
      // clang-format on
   };


   namespace NPrivate {
      constexpr std::string_view DefaultErrMsgPrefix = "Failed to ";
      template <class TString, class TStringView, class TErr>
      TString ErrorMessageFrom(TStringView Prefix, const char *Function, const int Line, TErr &&Err) {
         return TString(Prefix) + Function + " @ Line:" + std::to_string(Line) + ": " +
                std::forward<TErr>(Err);
      }
   }   // namespace NPrivate

   template <class COk, class CErr>
   class TCoResult;

   template <class CErr>
   class [[nodiscard]] TCoResult<void, CErr>: public TResult<void, CErr> {
      using TBase = TResult<void, CErr>;
      struct TPromise;

   public:
      using TBase::TBase;
      using TErr         = typename TBase::TErr;
      using promise_type = TPromise;

   private:
      using THandle = std::coroutine_handle<TPromise>;
      class TReturn {
      public:
         TReturn(const TReturn &) = delete;
         TReturn &operator=(const TReturn &) = delete;
         TReturn(TReturn &&)                 = delete;
         TReturn &operator=(TReturn &&) = delete;
         TReturn(THandle handle) noexcept {
            assert(handle);
            Handle = handle;
         }
         ~TReturn() noexcept {
            Handle.destroy();
         }
         THandle Handle;
      };

      struct TPromise {
         TReturn get_return_object() noexcept {
            return {THandle::from_promise(*this)};
         }
         std::suspend_never initial_suspend() noexcept {
            return {};
         }
         std::suspend_always final_suspend() noexcept {
            return {};
         }
         TResult<void, TErr> ReturnResult;
         template <ConvertibleTo<TErr> T>
         void return_error_value(T &&Error) noexcept {
            // From await_suspend => We know it must be an rvalue, hence move()
            ReturnResult = std::move(Error);
         }
         template <class T>
         void return_value(T &&Something) noexcept {   // from co_return expr
            ReturnResult = std::forward<T>(Something);
         }
         // For being ablt to write: co_return {};
         void return_value(std::initializer_list<int>) noexcept {}
         void unhandled_exception() noexcept {
            std::terminate();
         }
      };

      template <class TExternalErr>
      struct [[nodiscard]] TInternalAwaitabler {   // Awaiter + Awaitable
         std::optional<TExternalErr> ErrorValue;

         bool await_ready() noexcept {
            return ErrorValue.has_value() == false;
         }
         template <class TExternalHandle>
         void await_suspend(TExternalHandle Handle) noexcept {
            // Internal Result<> was an Err => Return control-flow to the caller
            assert(!await_ready());
            Handle.promise().return_error_value(std::move(*ErrorValue));
         }
         void await_resume() noexcept {
            assert(await_ready());
         }
      };

   public:
      TCoResult(TReturn && Return) noexcept: TBase(std::move(Return.Handle.promise().ReturnResult)) {}

      // Each Company has its own implementation of string
      template <class TString = std::string, class TStringView = std::string_view>
      requires StringPrependable<TString, TErr> TInternalAwaitabler<TString> OrPrependErrMsgAndReturn(
          TStringView Prefix   = NPrivate::DefaultErrMsgPrefix,
          const char *Function = __builtin_FUNCTION(),   // Replace with std::source_location
          const int   Line     = __builtin_LINE()) noexcept {
         if(this->IsOk()) {
            return {};
         } else {
            return {NPrivate::ErrorMessageFrom<TString>(Prefix, Function, Line, this->Err())};
         }
      }
      template <CallableCanGenerateNewErrFromOldErr<TErr> TCreateNewErr>
      auto OrReturnNewErr(TCreateNewErr && CreateNewErr) noexcept {
         using TRet = TInternalAwaitabler<std::decay_t<std::invoke_result_t<TCreateNewErr, TErr &&>>>;
         if(this->IsOk()) {
            return TRet {};
         } else {
            return TRet {CreateNewErr(std::move(this->Err()))};
         }
      }
      template <class T>
      TInternalAwaitabler<std::decay_t<T>> OrReturn(T && Something) noexcept {
         if(this->IsOk()) {
            return {};
         } else {
            return {std::forward<T>(Something)};
         }
      }
   };

   template <class COk, class CErr>
   class [[nodiscard]] TCoResult: public TResult<COk, CErr> {
      using TBase = TResult<COk, CErr>;
      struct TPromise;

   public:
      using TBase::TBase;
      using TOk          = typename TBase::TOk;
      using TErr         = typename TBase::TErr;
      using promise_type = TPromise;

   private:
      using THandle = std::coroutine_handle<TPromise>;
      class TReturn {
      public:
         TReturn(const TReturn &) = delete;
         TReturn &operator=(const TReturn &) = delete;
         TReturn(TReturn &&)                 = delete;
         TReturn &operator=(TReturn &&) = delete;
         TReturn(THandle handle) {
            assert(handle);
            Handle = handle;
         }
         ~TReturn() {
            Handle.destroy();
         }
         THandle Handle;
      };

      struct TPromise {
         TReturn get_return_object() noexcept {
            return {THandle::from_promise(*this)};
         }
         std::suspend_never initial_suspend() noexcept {
            return {};
         }
         std::suspend_always final_suspend() noexcept {
            return {};
         }
         TResult<TOk, TErr> ReturnResult;
         template <ConvertibleTo<TErr> T>
         void return_error_value(T &&Error) noexcept {
            // From await_suspend => We know it must be an rvalue, hence move()
            ReturnResult = ErrRes(std::move(Error));
         }
         template <class T>
         void return_value(T &&Something) noexcept {   // from co_return expr
            ReturnResult = std::forward<T>(Something);
         }
         void unhandled_exception() noexcept {
            std::terminate();
         }
      };

      template <class TExternalErr>
      struct [[nodiscard]] TCommonInternalAwaitabler {   // Awaiter + Awaitable
         std::variant<TOk *, TExternalErr> OkRefOrErrorValue;
         template <class... Ts>
         TCommonInternalAwaitabler(Ts && ... Vs): OkRefOrErrorValue(std::forward<Ts>(Vs)...) {}

         bool await_ready() noexcept {
            return OkRefOrErrorValue.index() == NPrivate::OkIndex;
         }
         template <class TExternalHandle>
         void await_suspend(TExternalHandle Handle) noexcept {
            // Internal Result<> was an Err => Return control-flow to the caller
            assert(!await_ready());
            Handle.promise().return_error_value(std::move(std::get<NPrivate::ErrIndex>(OkRefOrErrorValue)));
         }
      };
      // clang-format off
      template <class TExternalErr>
      struct [[nodiscard]] TLValueInternalAwaitabler: public TCommonInternalAwaitabler<TExternalErr> {
         template <class... Ts>
         TLValueInternalAwaitabler(Ts&&... Vs):
            TCommonInternalAwaitabler<TExternalErr>(std::forward<Ts>(Vs)...) {}
         TOk &await_resume() noexcept {   // Result is Ok => Return its Ok() and continue execution
            assert(this->await_ready());
            return *std::get<NPrivate::OkIndex>(this->OkRefOrErrorValue);
         }
      };
      template <class TExternalErr>
      struct [[nodiscard]] TRValueInternalAwaitabler: public TCommonInternalAwaitabler<TExternalErr> {
         template <class... Ts>
         TRValueInternalAwaitabler(Ts&&... Vs):
            TCommonInternalAwaitabler<TExternalErr>(std::forward<Ts>(Vs)...) {}
         TOk &&await_resume() noexcept {   // Result is Ok => Return its Ok() and continue execution
            assert(this->await_ready());
            return std::move(*std::get<NPrivate::OkIndex>(this->OkRefOrErrorValue));
         }
      };
      // clang-format on

   public:
      TCoResult(TReturn &&Return) noexcept: TBase(std::move(Return.Handle.promise().ReturnResult)) {}

      // Each Company has its own implementation of string
      template <class TString = std::string, class TStringView = std::string_view>
      requires StringPrependable<TString, TErr> TLValueInternalAwaitabler<TString> OrPrependErrMsgAndReturn(
          TStringView Prefix   = NPrivate::DefaultErrMsgPrefix,
          const char *Function = __builtin_FUNCTION(),   // Replace with std::source_location
          const int   Line     = __builtin_LINE()) & noexcept {
         if(this->IsOk()) {
            return {std::in_place_index_t<0>(), &this->Ok()};
         } else {
            return {std::in_place_index_t<1>(),
                    NPrivate::ErrorMessageFrom<TString>(Prefix, Function, Line, this->Err())};
         }
      }
      template <class TString = std::string, class TStringView = std::string_view>
      requires StringPrependable<TString, TErr> TRValueInternalAwaitabler<TString> OrPrependErrMsgAndReturn(
          TStringView Prefix   = NPrivate::DefaultErrMsgPrefix,
          const char *Function = __builtin_FUNCTION(),   // Replace with std::source_location
          const int   Line     = __builtin_LINE()) && noexcept {
         if(this->IsOk()) {
            return {std::in_place_index_t<0>(), &this->Ok()};
         } else {
            return {std::in_place_index_t<1>(),
                    NPrivate::ErrorMessageFrom<TString>(Prefix, Function, Line, this->Err())};
         }
      }
      template <CallableCanGenerateNewErrFromOldErr<TErr> TCreateNewErr>
      auto OrReturnNewErr(TCreateNewErr &&CreateNewErr) & noexcept {
         using TRet = TLValueInternalAwaitabler<std::decay_t<std::invoke_result_t<TCreateNewErr, TErr &&>>>;
         if(this->IsOk()) {
            return TRet {std::in_place_index_t<0>(), &this->Ok()};
         } else {
            return TRet {std::in_place_index_t<1>(), CreateNewErr(std::move(this->Err()))};
         }
      }
      template <CallableCanGenerateNewErrFromOldErr<TErr> TCreateNewErr>
      auto OrReturnNewErr(TCreateNewErr &&CreateNewErr) && noexcept {
         using TRet = TRValueInternalAwaitabler<std::decay_t<std::invoke_result_t<TCreateNewErr, TErr &&>>>;
         if(this->IsOk()) {
            return TRet {std::in_place_index_t<0>(), &this->Ok()};
         } else {
            return TRet {std::in_place_index_t<1>(), CreateNewErr(std::move(this->Err()))};
         }
      }
      template <class T>
      TLValueInternalAwaitabler<std::decay_t<T>> OrReturn(T &&Something) & noexcept {
         if(this->IsOk()) {
            return {std::in_place_index_t<0>(), &this->Ok()};
         } else {
            return {std::in_place_index_t<1>(), std::forward<T>(Something)};
         }
      }
      template <class T>
      TRValueInternalAwaitabler<std::decay_t<T>> OrReturn(T &&Something) && noexcept {
         if(this->IsOk()) {
            return {std::in_place_index_t<0>(), &this->Ok()};
         } else {
            return {std::in_place_index_t<1>(), std::forward<T>(Something)};
         }
      }
   };
}   // namespace NDiRes
