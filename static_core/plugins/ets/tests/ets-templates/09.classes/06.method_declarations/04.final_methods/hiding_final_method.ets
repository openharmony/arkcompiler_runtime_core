/*---
desc: Final method cannot be hidden
tags: [compile-only, negative]
---*/

class A {
  final method(): void {}
}

class B extends A {
  method(): void {}
}