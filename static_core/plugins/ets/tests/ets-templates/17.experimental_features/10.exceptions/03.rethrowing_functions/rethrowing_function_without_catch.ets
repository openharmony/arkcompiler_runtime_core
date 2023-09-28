/*---
desc: Rethrowing function declaration without catch.
assert: A function that that accepts a throwing function as a parameter can be marked with the rethrows keyword. Such function is called a rethrowing function.
tags: [compile-only, negative]
---*/

class TestException extends Exception {}
type func = (): void;

function foo(bar: func throws): void rethrows {
  bar();
}

function canThrow (): void {
  throw new TestException();
}

function main(): void {
  foo(canThrow);
}