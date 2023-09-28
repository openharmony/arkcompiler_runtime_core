/*---
desc: Declaring throwing function.
assert: To indicate that a function (in general sense, meaning a function, method, constructor or lambda) can throw an error, use the throws keyword at the end of the signature. A function marked with throws is called a throwing function. A function type can also be marked as ‘throws’.
tags: [compile-only]
---*/

function bar(): void throws {
  throw new Exception();
}
