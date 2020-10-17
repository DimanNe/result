[![CMake Actions Status](https://github.com/dimanne/result/workflows/CMake/badge.svg)](https://github.com/dimanne/result/actions)

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
Before:
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

After:
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

[Here](https://github.com/DimanNe/scripts/tree/master/backup) is a real/larger project that uses `TCoResult<>`.


##### A real-world example of before-after:
Before:
```
TResult<TTerm, std::string> BeginTermOr = ExtractInteger<TTerm>(Labels, BeginTermKey());
if(BeginTermOr.IsErr()) {
   std::ostringstream str;
   str << "Failed while deserialising TBlobInfo from object labels: " << BeginTermOr.Err();
   return str.str();
}
TResult<TTerm, std::string> EndTermOr = ExtractInteger<TTerm>(Labels, EndTermKey());
if(EndTermOr.IsErr()) {
   std::ostringstream str;
   str << "Failed while deserialising TBlobInfo from object labels: " << EndTermOr.Err();
   return str.str();
}
```

After:
```
const TTerm BeginTerm = co_await ExtractInteger<TTerm>(Labels, BeginTermKey()).OrPrependErrMsgAndReturn();
const TTerm EndTerm   = co_await ExtractInteger<TTerm>(Labels, EndTermKey()).OrPrependErrMsgAndReturn();
```
Note: while error message in the latter case will be different, it will contain all required information,
in particular instead of "*Failed while deserialising TBlobInfo from object labels: ...*" it will be:
"*Failed to get BlobInfoFromGoogleCloud @ Line 123: ...*"



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

##### Known limitations
* The `co_await` approach works only when you want to **propagate error** by returning control-flow
  from a function to its caller. In other words, if you have a loop and want to accumulate/remember
  all errors (and use `continue`) you need to do it in the "normal" way.
* If you use `co_await`, and later in your function you want to `return`, you have to
  use `co_return` instead.

##### What if I forget to call co_await?
You will not, struct returned by `Or...Return...` functions is marked with `[[nodiscard]]` attribute,
so you will get (at least) a compiler warning.

##### What if I assign temporary Result to an (lvalue) reference?
In this case:
```
TCoResult<std::string, int> DoSomething();
TCoResult<std::string, int> GetSomething() {
   std::string & DANGLING_REFERENCE = co_await DoSomething().OrReturn(42);
}
```
you will get a compiler error.
[Commit](https://github.com/DimanNe/result/commit/171318571ed08930a339170746f670589da99b35) with the feature.


</p></details>


### `Result<Ok, Err>`

If you are wondering what the problems with exceptions are, you can start, for example,
[here](https://www.reddit.com/r/cpp/comments/cliw5j/should_not_exceptions_be_finally_deprecated/).

