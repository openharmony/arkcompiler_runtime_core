/*---
desc: Abstract method declaraion.
assert: The declaration of an abstract method m must appear directly within an abstract class; otherwise a compile-time error occurs.
tags: [compile-only, negative]
---*/

class A {
  abstract run(): void;
}
