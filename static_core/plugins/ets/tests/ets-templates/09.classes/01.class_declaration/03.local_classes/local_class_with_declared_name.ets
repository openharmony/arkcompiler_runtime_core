/*---
desc: Declaring local class in function body with declared name.
assert: A compile-time error occurs if a local class or interface declaration has a name that is used to declare a new local class or interface (unless such new local class or interface is declared within a class or interface declaration).
tags: [compile-only, negative]
---*/

function bar(): void {
  let B: int = 1;

  class B {
    public x: int;
  }
}