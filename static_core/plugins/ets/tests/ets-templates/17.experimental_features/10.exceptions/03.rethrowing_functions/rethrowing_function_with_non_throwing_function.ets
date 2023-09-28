/*---
desc: Rethrowing function declaration with exception parameter.
assert: When calling a rethrowing function R its argument can be both throwing and non-throwing function. 
tags: [compile-only]
---*/

class TestException extends Exception {}
type func = () => void;

function foo(bar: func throws): void rethrows {
  bar();
}

function foo(): int {
  return 5;
}

function main(): void {
  assert bar(foo) == 5;
}