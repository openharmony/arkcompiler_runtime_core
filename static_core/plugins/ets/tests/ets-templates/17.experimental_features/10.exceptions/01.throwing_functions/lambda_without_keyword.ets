/*---
desc: Declaring throwing lambda without 'throws' keyword.
assert: To indicate that a function (in general sense, meaning a function, method, constructor or lambda) can throw an error, use the throws keyword at the end of the signature. A function marked with throws is called a throwing function. A function type can also be marked as ‘throws’.
tags: [compile-only, negative]
---*/

class TestException extends Exception {}

type func = (x: int) => int;

function bar(): void {
  let foo: func = (x: int): int => { throw new TestException(); return 5; };

  let i = foo(5);
}

function main(): void {
  bar();
}
