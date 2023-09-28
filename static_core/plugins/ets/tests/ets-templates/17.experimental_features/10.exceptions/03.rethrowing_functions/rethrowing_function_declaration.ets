/*---
desc: Rethrowing function declaration.
assert: A function that that accepts a throwing function as a parameter can be marked with the rethrows keyword. Such function is called a rethrowing function.
tags: [compile-only]
---*/

type func = () => int;

function foo(bar: func throws): void rethrows {
  bar();
}
