/*---
desc: Declaring public local class in function body.
assert: A local class is not compatible with the access modifiers public and static.
tags: [compile-only, negative]
---*/

function bar(): void {
  public inner class B {}
}