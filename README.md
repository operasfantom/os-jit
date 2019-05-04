#  Intro

Inspired by http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1609r0.html. 
Complex attempt to implement simplified process of jit compilation for void-template function without use of `libdl`. Unfortunatelly, it doesn't work.

## Details
```JIT_DECLARATION``` create string literal with kept code
```JIT_CALL``` lazily compile template specialization to shared library and load it.
## Requirements
* Gcc or Clang (C++17 or higher)
* Cmake 3.12 or higher

## Usage
 * To declare function that could be possibly jit'ed write
 ```
 JIT_DECLARATION
 (
 		fun,
 		template<template params>
 		void fun(args...) {
 
 		}
 )
 ```
 * To run
 ```
 JIT_CALL(fun, fun<template params>, args...);
 ```
 
## Todo
 * maintain non-zero count of function args.
 * returning value of jit function
 * find mangled name in shared library and call it directly.