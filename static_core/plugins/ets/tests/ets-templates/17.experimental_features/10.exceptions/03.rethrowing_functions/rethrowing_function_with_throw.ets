/*---
desc: Rethrowing function declaration with throw statement.
assert: A rethrowing function could not contain a throw statement. If the function body contains a throw statement it should be marked with throws keyword, not with rethrows.
tags: [compile-only, negative]
---*/

type func = (): void;

function foo(bar: func throws): void rethrows {
  throw new Exception();
}