/*---
desc: Final overrided method version
tags: [compile-only, negative]
---*/

class A {
  method(): void {}
}

class B extends A {
  final override method(): void {}
}

class C extends B {
  override method(): void {}
}