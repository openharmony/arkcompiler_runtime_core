/*---
desc: Final method cannot be overridden
tags: [compile-only, negative]
---*/

class A {
  final method(): void {}
}

class B extends A {
  override method(): void {}
}