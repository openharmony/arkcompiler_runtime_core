/*---
desc: Declaring local class in function body with static member.
assert: A local class is an inner class that is not implicitly static. Local classes are non-static and cannot contain most kinds of static declarations.
tags: [compile-only, negative]
---*/

function bar(): void {
  class B {
    public static x: int;
  }
}