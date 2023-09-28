/*---
desc: Abstract method declaraion with private access modifier.
assert: It is compile-time error if an abstract method is marked as private.
tags: [negative, compile-only]
---*/

class A {
  private abstract run(): void;
}
