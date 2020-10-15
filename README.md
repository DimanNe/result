This repo contains `Result<Ok, Err>` (which is not very interesting, tbh) and **CoResult<Ok, Err> which
is more interesting** as it exploits C++20 coroutines to:
1. to eradicate boilerplate associated with the Result-based approach to error-handling,
2. and to make it impossible to misuse `Result<>` - in particular, to forget to check what it contains
   (`Error` or `Ok`) before accessing it (which results in exceptions/crashes/UB).

Since the latter (`CoResult`) is more interesting, let's start with it:

### `CoResult<Ok, Err>`

An always-up-to-date example can be found [here](https://github.com/DimanNe/result/blob/master/examples/main.cpp).
