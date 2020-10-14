#pragma once

#include "result.h"

#include <cassert>
#include <experimental/coroutine>

namespace NDiRes {

   template <class T>
   concept StringPrependable = requires(T a) {
      std::string() + a;
   };

   template <class TFrom, class TTo>
   concept ConvertibleTo =
       std::is_convertible_v<TFrom, TTo> &&requires(std::add_rvalue_reference_t<TFrom> (&f)()) {
      static_cast<TTo>(f());
   };

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
      using THandle = std::experimental::coroutine_handle<TPromise>;
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
         std::experimental::suspend_never initial_suspend() noexcept {
            return {};
         }
         std::experimental::suspend_always final_suspend() noexcept {
            return {};
         }
         TResult<void, TErr> ReturnResult;
         template <class T>
         void return_error_value(T &&Error) noexcept {
            // From await_suspend => We know it must be an rvalue, hence move()
            ReturnResult = std::move(Error);
         }
         void return_void() noexcept {}
         void unhandled_exception() {
            std::terminate();
         }
      };

      struct [[nodiscard]] TInternalAwaitabler {   // Awaiter + Awaitable
         std::optional<TErr> ErrorValue;

         bool await_ready() noexcept {
            // Need to decide what to do: continue this function or return to caller
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

      TInternalAwaitabler OrPrependErrMsgAndReturn(
          std::string_view Prefix = "Failed to ",
          // The following builtins will be replaced with struct std::source_location
          // https://en.cppreference.com/w/cpp/experimental/source_location
          const char *Function = __builtin_FUNCTION(),
          const int   Line     = __builtin_LINE()) noexcept {
         if(this->IsOk()) {
            return {};
         } else {
            return {std::string(Prefix) + Function + " @ Line:" + std::to_string(Line) + ": " + this->Err()};
         }
      }
      template <class TCreateNewErr>
      TInternalAwaitabler OrReturnNewErr(TCreateNewErr && CreateNewErr) noexcept {
         if(this->IsOk()) {
            return {};
         } else {
            return {CreateNewErr(std::move(this->Err()))};
         }
      }
      template <class T>
      TInternalAwaitabler OrReturn(T && Something) noexcept {
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
      using THandle = std::experimental::coroutine_handle<TPromise>;
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
         std::experimental::suspend_never initial_suspend() noexcept {
            return {};
         }
         std::experimental::suspend_always final_suspend() noexcept {
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
         void unhandled_exception() {
            std::terminate();
         }
      };

      template <class TExternalErr>
      struct [[nodiscard]] TInternalAwaitabler {   // Awaiter + Awaitable
         std::variant<TOk *, TExternalErr> OkRefOrErrorValue;

         bool await_ready() noexcept {
            // Need to decide what to do: continue this function or return to caller
            return OkRefOrErrorValue.index() == NPrivate::OkIndex;
         }
         template <class TExternalHandle>
         void await_suspend(TExternalHandle Handle) noexcept {
            // Internal Result<> was an Err => Return control-flow to the caller
            assert(!await_ready());
            Handle.promise().return_error_value(std::move(std::get<NPrivate::ErrIndex>(OkRefOrErrorValue)));
         }
         TOk &await_resume() noexcept {   // Result is Ok => Return its Ok() and continue execution
            assert(await_ready());
            return *std::get<NPrivate::OkIndex>(OkRefOrErrorValue);
         }
      };

   public:
      TCoResult(TReturn && Return) noexcept: TBase(std::move(Return.Handle.promise().ReturnResult)) {}

      TInternalAwaitabler<std::string> OrPrependErrMsgAndReturn(
          std::string_view Prefix = "Failed to ",
          // The following builtins will be replaced with struct std::source_location
          // https://en.cppreference.com/w/cpp/experimental/source_location
          const char *Function = __builtin_FUNCTION(),
          const int   Line     = __builtin_LINE()) noexcept {
         static_assert(
             StringPrependable<TErr>,
             "This function prepends string description to TErr type, which is not string-prependable");
         if(this->IsOk()) {
            return {.OkRefOrErrorValue {std::in_place_index_t<0>(), &this->Ok()}};
         } else {
            std::string NewErrMsg =
                std::string(Prefix) + Function + " @ Line:" + std::to_string(Line) + ": " + this->Err();
            return {.OkRefOrErrorValue {std::in_place_index_t<1>(), std::move(NewErrMsg)}};
         }
      }
      template <class TCreateNewErr>
      TInternalAwaitabler<std::decay_t<std::invoke_result_t<TCreateNewErr, TErr &&>>> OrReturnNewErr(
          TCreateNewErr && CreateNewErr) noexcept {
         if(this->IsOk()) {
            return {.OkRefOrErrorValue {std::in_place_index_t<0>(), &this->Ok()}};
         } else {
            return {.OkRefOrErrorValue {std::in_place_index_t<1>(), CreateNewErr(std::move(this->Err()))}};
         }
      }
      template <class T>
      TInternalAwaitabler<std::decay_t<T>> OrReturn(T && Something) noexcept {
         if(this->IsOk()) {
            return {.OkRefOrErrorValue {std::in_place_index_t<0>(), &this->Ok()}};
         } else {
            return {.OkRefOrErrorValue {std::in_place_index_t<1>(), std::forward<T>(Something)}};
         }
      }
   };

}   // namespace NDiRes
