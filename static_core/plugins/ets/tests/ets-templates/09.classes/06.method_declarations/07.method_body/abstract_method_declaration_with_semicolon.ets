/*---
desc: Abstract method declaraion with simply semicolon.
assert: The body of a method must be a semicolon if the method is abstract or native.
tags: [compile-only]
---*/

abstract class A {
  abstract run(): void;
}