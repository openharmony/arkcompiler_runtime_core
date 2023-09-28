/*---
desc: Declaring local class in function body with non-static member.
assert: A compile-time error occurs if a local class or interface declaration has a member variable that is not defined as static.
tags: [compile-only, negative]
---*/

function bar(): void {
  class B {
    public x: int;
  }
}