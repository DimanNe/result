#pragma once

#include <optional>
#include <ostream>
#include <tuple>
#include <variant>

namespace NDiRes {
   namespace NPrivate {
      constexpr size_t OkIndex  = 0;
      constexpr size_t ErrIndex = 1;
      template <size_t IndexToSet, class... Ts>
      struct TOkOrErrWrapper;
   }   // namespace NPrivate

   template <class COk, class CErr>
   class TResult;

   template <class CErr>
   class [[nodiscard]] TResult<void, CErr> {
      template <class T>
      static constexpr bool IsConvertibleToErr() {
         return std::is_convertible_v<T, CErr>;
      }

   public:
      using TErr  = CErr;
      using TSelf = TResult<void, TErr>;

      TResult() noexcept = default;
      template <class T>
      TResult(T && Err, std::enable_if_t<IsConvertibleToErr<T>()> * = nullptr) noexcept:
          Optional(std::in_place, std::forward<T>(Err)) {}

      bool operator==(const TSelf &Another) const noexcept {
         return Optional == Another.Optional;
      }
      bool operator!=(const TSelf &Another) const noexcept {
         return !(*this == Another);
      }

      bool IsErr() const noexcept {
         return static_cast<bool>(Optional);
      }
      bool IsOk() const noexcept {
         return IsErr() == false;
      }
      explicit operator bool() const noexcept {
         return IsOk();
      }

      [[nodiscard]] TErr &Err() noexcept {
         return *Optional;
      }
      [[nodiscard]] const TErr &Err() const noexcept {
         return *Optional;
      }

      /// Unfortunately, gtest/gmock accepts their arguments to compare via a const lvalue ref, which ruins
      /// all forwarding/moving optimisations.
      /// There two ways:
      ///    * either do not use EXPECT_EQ / EXPECT_NE macros, and use instead EXPECT_TRUE(a == b)
      ///    * or, alternatively, replace signature in such a way that it accept Wrapper as a const lvalue
      ///      ref, but then remove const and cast it to rvalue, as we know that the Wrapper is always a
      ///      result of OkRes/ErrRes function invocation and therefore (was) an rvalue.
      template <size_t IndexToSet, class... Ts>
      bool operator==(NPrivate::TOkOrErrWrapper<IndexToSet, Ts...> &&Wrapper) const noexcept {
         if constexpr(IndexToSet == NPrivate::OkIndex) {
            // We could return false; in the case below, but let's make it compiler error, in order to match
            // better with C++ behaviour, when one, for example, writes something like this:
            // int a = 2;
            // if(a == "qwer") ... compiler will produce an error, so will we
            static_assert(sizeof...(Ts) == 0, "You are trying to compare void with a value of non-void type");
            return IsOk();
         } else {   // IndexToSet == NPrivate::ErrIndex
            // We Compare TResult<void, TErr> with ErrRes() or ErrRes(Something)
            if constexpr(sizeof...(Ts) == 1)
               return IsErr() && Err() == std::get<0>(Wrapper.Tuple);
            else   // We were given several arguments => contruct TErr type out of them and compare
               return IsErr() &&
                      Err() ==
                         std::apply([](auto &&... Vs) { return TErr {std::forward<decltype(Vs)>(Vs)...}; },
                                    std::move(Wrapper.Tuple));
         }
      }
      template <size_t IndexToSet, class... Ts>
      bool operator!=(NPrivate::TOkOrErrWrapper<IndexToSet, Ts...> &&Wrapper) noexcept {
         return !(*this == std::move(Wrapper));
      }

   private:
      std::optional<TErr> Optional;
      template <size_t IndexToSet, class... Ts>
      friend struct NPrivate::TOkOrErrWrapper;
   };


   template <class COk, class CErr>
   class [[nodiscard]] TResult {
   public:
      using TOk   = COk;
      using TErr  = CErr;
      using TSelf = TResult<TOk, TErr>;

   private:
      template <class T>
      static constexpr bool IsConvertibleToBoth() {
         return std::is_convertible_v<T, TOk> && std::is_convertible_v<T, TErr>;
      }
      template <class T>
      static constexpr bool IsConvertibleOnlyToErr() {
         return std::is_convertible_v<T, TErr> && !IsConvertibleToBoth<T>();
      }
      template <class T>
      static constexpr bool IsConvertibleOnlyToOk() {
         return std::is_convertible_v<T, TOk> && !IsConvertibleToBoth<T>();
      }

   public:
      template <class T>
      TResult(T && ok, std::enable_if_t<IsConvertibleOnlyToOk<T>()> * = nullptr) noexcept:
         Storage(std::forward<T>(ok)) {}

      template <class T>
      TResult(T && err, std::enable_if_t<IsConvertibleOnlyToErr<T>()> * = nullptr) noexcept:
          Storage(std::forward<T>(err)) {}

      bool operator==(const TSelf &Another) const noexcept {
         return Storage == Another.Storage;
      }
      bool operator!=(const TSelf &Another) const noexcept {
         return !(*this == Another);
      }

      bool IsOk() const noexcept {
         return Storage.index() == NPrivate::OkIndex;
      }
      bool IsErr() const noexcept {
         return Storage.index() == NPrivate::ErrIndex;
      }
      explicit operator bool() const noexcept {
         return IsOk();
      }

      [[nodiscard]] TOk &Ok() noexcept {
         return std::get<0>(Storage);
      }
      [[nodiscard]] const TOk &Ok() const noexcept {
         return std::get<0>(Storage);
      }
      [[nodiscard]] TOk &operator*() noexcept {
         return Ok();
      }
      [[nodiscard]] const TOk &operator*() const noexcept {
         return Ok();
      }
      [[nodiscard]] TOk *operator->() noexcept {
         return &Ok();
      }
      [[nodiscard]] const TOk *operator->() const noexcept {
         return &Ok();
      }

      [[nodiscard]] TErr &Err() noexcept {
         return std::get<1>(Storage);
      }
      [[nodiscard]] const TErr &Err() const noexcept {
         return std::get<1>(Storage);
      }


      /// Unfortunately, gtest/gmock accepts their arguments to compare via a const lvalue ref, which ruins
      /// all forwarding/moving optimisations.
      /// There two ways:
      ///    * either do not use EXPECT_EQ / EXPECT_NE macros, and use instead EXPECT_TRUE(a == b)
      ///    * or, alternatively, replace signature in such a way that it accept Wrapper as a const lvalue
      ///      ref, but then remove const and cast it to rvalue, as we know that the Wrapper is always a
      ///      result of OkRes/ErrRes function invocation and therefore (was) an rvalue.
      template <size_t IndexToSet, class... Ts>
      bool operator==(NPrivate::TOkOrErrWrapper<IndexToSet, Ts...> &&Wrapper) const noexcept {
         if constexpr(IndexToSet == NPrivate::OkIndex) {
            if constexpr(sizeof...(Ts) == 1)
               return IsOk() && Ok() == std::get<0>(Wrapper.Tuple);
            else
               return IsOk() &&
                      Ok() ==
                         std::apply([](auto &&... Vs) { return TOk {std::forward<decltype(Vs)>(Vs)...}; },
                                    std::move(Wrapper.Tuple));
         } else {   // IndexToSet == NPrivate::ErrIndex
            // We Compare TResult<void, TErr> with ErrRes() or ErrRes(Something)
            if constexpr(sizeof...(Ts) == 1)
               return IsErr() && Err() == std::get<0>(Wrapper.Tuple);
            else
               return IsErr() &&
                      Err() ==
                         std::apply([](auto &&... Vs) { return TErr {std::forward<decltype(Vs)>(Vs)...}; },
                                    std::move(Wrapper.Tuple));
         }
      }
      template <size_t IndexToSet, class... Ts>
      bool operator!=(NPrivate::TOkOrErrWrapper<IndexToSet, Ts...> &&Wrapper) const noexcept {
         return !(*this == std::move(Wrapper));
      }

   private:
      TResult() noexcept = default;
      std::variant<TOk, TErr> Storage;
      template <size_t IndexToSet, class... Ts>
      friend struct NPrivate::TOkOrErrWrapper;
   };

   template <class TErr>
   using TVoidResult = TResult<void, TErr>;

   template <class TOk, class TErr>
   std::ostream &operator<<(std::ostream &Out, const TResult<TOk, TErr> &Result) noexcept {
      if constexpr(std::is_same_v<TOk, void>) {
         if(Result)
            Out << "Success";
         else
            Out << Result.Err();
      } else {
         if(Result)
            Out << "Ok(" << *Result << ")";
         else
            Out << "Err(" << Result.Err() << ")";
      }
      return Out;
   }


   namespace NPrivate {
      template <class...>
      constexpr bool PostponeStaticAssert = false;

      template <size_t IndexToSet, class... Ts>
      struct TOkOrErrWrapper {
         TOkOrErrWrapper(Ts &&... Vs) noexcept: Tuple(std::forward<Ts>(Vs)...) {}

         template <class Ok, class Err>
         operator TResult<Ok, Err>() &&noexcept {
            if constexpr(std::is_same_v<Ok, void>) {
               if constexpr(IndexToSet == NPrivate::OkIndex) {
                  // Was called from  NPrivate::TOkOrErrWrapper<0> OkRes()
                  static_assert(sizeof...(Ts) == 0,
                                "You attempted to create Result<void, Err> representing success (void), but "
                                "also specified argument(s) in OkRes(x) for the void type");
                  return {};   // Return Ok Result
               } else
                  return std::apply(
                     [](auto &&... Vs) {
                        TResult<Ok, Err> Result;
                        Result.Optional.template emplace(std::forward<decltype(Vs)>(Vs)...);
                        return Result;
                     },
                     std::move(Tuple));
            } else
               return std::apply(
                  [](auto &&... Vs) {
                     TResult<Ok, Err> Result;
                     Result.Storage.template emplace<IndexToSet>(std::forward<decltype(Vs)>(Vs)...);
                     return Result;
                  },
                  std::move(Tuple));
         }
         template <class Ok, class Err>
         operator TResult<Ok, Err>() &noexcept {
            static_assert(
                PostponeStaticAssert<Ok, Err>,
                "This type can be produced only by OkRes()/ErrRes() helper function, and is supposed to be "
                "immediately consumed by/assigned to a Result<>, do not store it in other variables");
         }

         template <class Ok, class Err>
         bool operator==(const TResult<Ok, Err> &r) &&noexcept {
            return r == std::move(*this);
         }
         template <class Ok, class Err>
         bool operator==([[maybe_unused]] const TResult<Ok, Err> &r) &noexcept {
            static_assert(
                PostponeStaticAssert<Ok, Err>,
                "This type can be produced only by OkRes()/ErrRes() helper function, and is supposed to be "
                "immediately consumed by/assigned to a Result<>, do not store it in other variables");
         }
         template <class Ok, class Err>
         bool operator!=(const TResult<Ok, Err> &r) &&noexcept {
            return !(std::move(*this) == r);
         }
         template <class Ok, class Err>
         bool operator!=([[maybe_unused]] const TResult<Ok, Err> &r) &noexcept {
            static_assert(
                PostponeStaticAssert<Ok, Err>,
                "This type can be produced only by OkRes()/ErrRes() helper function, and is supposed to be "
                "immediately consumed by/assigned to a Result<>, do not store it in other variables");
         }

         std::tuple<Ts &&...> Tuple;
      };



   }   // namespace NPrivate

   inline NPrivate::TOkOrErrWrapper<NPrivate::OkIndex> OkRes() noexcept {   /// For TVoidRes
      return {};
   }
   // template <class X>
   // NPrivate::TOkOrErrWrapper<0, X> OkRes(X &&x) noexcept {
   //    return {std::forward<X>(x)};
   // }
   template <class... Ts>
   NPrivate::TOkOrErrWrapper<NPrivate::OkIndex, Ts...> OkRes(Ts &&... Vs) noexcept {
      return {std::forward<Ts>(Vs)...};
   }
   // template <class X>
   // NPrivate::TOkOrErrWrapper<1, X> ErrRes(X &&x) noexcept {
   //    return {std::forward<X>(x)};
   // }
   template <class... Ts>
   NPrivate::TOkOrErrWrapper<NPrivate::ErrIndex, Ts...> ErrRes(Ts &&... Vs) noexcept {
      return {std::forward<Ts>(Vs)...};
   }
}   // namespace NDiRes

namespace std {
   template <class TOk, class TErr>
   struct hash<NDiRes::TResult<TOk, TErr>> {
      std::size_t operator()(const NDiRes::TResult<TOk, TErr> &Result) const {
         if constexpr(std::is_same_v<TOk, void>) {
            return std::hash<std::optional<TErr>>()(Result.Optional);
         } else {
            return std::hash<std::variant<TOk, TErr>>()(Result.Storage);
         }
      }
   };
}   // namespace std
