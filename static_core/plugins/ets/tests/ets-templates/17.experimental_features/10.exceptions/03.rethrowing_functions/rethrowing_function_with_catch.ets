/*---
desc: Rethrowing function declaration with exception parameter.
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
  let y: int = 1;
  try() {
    foo(canThrow);
  }
  catch(e: TestException) {
    y = 5;
  }
  catch() {
    y = 10;
  }
  assert y == 5;
}