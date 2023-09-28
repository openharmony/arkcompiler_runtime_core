/*---
desc: Declaring static local class in function body.
assert: A local class is not compatible with the access modifiers public and static.
tags: [compile-only, negative]
---*/

function bar(): void {
  static inner class B {}
}