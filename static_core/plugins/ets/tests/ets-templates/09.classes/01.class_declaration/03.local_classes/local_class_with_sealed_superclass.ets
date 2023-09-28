/*---
desc: Declaring local class in function body with sealed superclass.
assert: A compile-time error occurs if the direct superclass or a direct superinterface of a local class is sealed.
tags: [compile-only, negative]
---*/

sealed class A {}

function bar(): void {
  class B extends A {
    public bar(): void {
      let y: int = x + 1;
    }
  }
}