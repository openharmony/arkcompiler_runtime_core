/*---
desc: Rethrowing function declaration with default exception.
assert: A function that that accepts a throwing function as a parameter can be marked with the rethrows keyword. Such function is called a rethrowing function.
tags: [compile-only, negative]
---*/

type func = (): void;

function foo(bar: func throws): void rethrows {
  bar();
}

function canThrow (): void {
  throw new Exception();
}

function main(): void {
  let y: int = 1;
  try() {
    foo(canThrow);
  }
  catch() {
    y = 5;
  }
  assert y == 5;
}