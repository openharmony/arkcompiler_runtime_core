/*---
desc: Overriding abstract method by abstract method.
tags: [compile-only, negative]
---*/

class A {
  open run(): void {
    let result = 5 + 5;
  };
}

abstract class B extends A {
  abstract run(): void;
}
