This repo contains `Result<Ok, Err>` (which is not very interesting, tbh) and **CoResult<Ok, Err> which
is more interesting** as it exploits C++20 coroutines:
1. to eradicate boilerplate associated with the Result-based approach to error-handling,
2. and, more importantly, to make it impossible to misuse `Result<>` - in particular, to forget to
   check what it contains (`Error` or `Ok`) before accessing it (which results in exceptions/crashes/UB).

Since the latter (`CoResult`) is more interesting, let's start with it:

### `CoResult<Ok, Err>`

First of all, an always-up-to-date example can be found
[here](https://github.com/DimanNe/result/blob/master/examples/main.cpp).

Secondly, here is before-after:
##### Before:
```
/// Returns intefer on success, or explanation in std::string in case of an error
TResult<int, std::string> ParseNumber(std::string_view FileName);

TResult<int, std::string> ReadSettings(std::string_view FileName) {
   TResult<int, std::string> IntOrError = ParseNumber(FileName);
   if(IntOrError.IsErr()) // Burdensome, error-prone, what if we forget to check it?
      return IntOrError.Err();
   int AdjustedSettings = std::max(1234, IntOrError.Ok());
   return AdjustedSettings;
}
```

##### After:
```
TResult<int, std::string> ParseNumber(std::string_view FileName);

TResult<int, std::string> ReadSettings(std::string_view FileName) {
   int Int = co_await ParseNumber(FileName).OrPrependErrMsgAndReturn();
   return std::max(1234, Int);
}
```

if we wanted we could shorten the "After version" to this:
```
TResult<int, std::string> ReadSettings(std::string_view FileName) {
   return std::max(1234, co_await ParseNumber(FileName).OrPrependErrMsgAndReturn());
}
```


**In other words, the following:**
```
TOk Ok = co_await ExpressionOfTypeResult<TOk, TErr> . OrPrependErrMsgAndReturn();
```
**will** (depending on the contents of the Result expression)
* either **extract Ok value from it and assign it to the variable on the left**
* or **return from the function** what is specified as parameter of `OrPrependErrMsgAndReturn()` function.
  Note, in the implementation, there is a family of `Or..Return...` functions: (1) `OrPrependErrMsgAndReturn(...)`
  (2) `OrReturnNewErr(...)`, (3) `OrReturn(...)`.


##### More features / description:
<details><summary>... if you wish</summary><p>

##### No redundant/temporary Result<void, TErr> variables
For `TResult<void, TErr>` you no longer need to create a variable that would hold the result (only
to append error explanation later):

Before:
```
// RenameResult is needed only because it holds Error (in case of error)
TResult<void, std::string> RenameResult = Rename(Old, New);
if(RenameResult.IsErr()) {
   std::string NewError = "Failed to rename from " + Old + " to " + New + " " + RenameResult.Err();
   return NewError;
}
```
After:
```
co_await Rename(Old, New).OrReturnNewErr([&](std::string &&Err) {
   return "Failed to rename from " + Old + " to " + New + " " + Err;
});
```

</p></details>


### `Result<Ok, Err>`

If you are wondering what the problems with exceptions are, you can start, for example,
[here](https://www.reddit.com/r/cpp/comments/cliw5j/should_not_exceptions_be_finally_deprecated/).

