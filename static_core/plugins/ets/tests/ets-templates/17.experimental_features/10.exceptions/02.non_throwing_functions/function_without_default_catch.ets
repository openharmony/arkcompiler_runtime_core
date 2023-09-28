/*---
desc: Declaring non-throwing function without default catch.
assert: To indicate that a function (in general sense, meaning a function, method, constructor or lambda) can throw an error, use the throws keyword at the end of the signature. A function marked with throws is called a throwing function. A function type can also be marked as ‘throws’.
tags: [compile-only, negative]
---*/

class TestException extends Exception {}

function bar(): int {
  try {
    throw new TestException();
    return 1;
  }
  catch (e: TestException) {
    return 5;
  }

  return 2;
}

function main(): void {
  let i: int = bar();
}