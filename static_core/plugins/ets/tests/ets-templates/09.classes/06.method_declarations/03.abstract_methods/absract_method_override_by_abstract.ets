/*---
desc: Overriding abstract method.
assert: An abstract class can override an abstract method by providing another abstract method declaration.
tags: [compile-only]
---*/

abstract class A {
  abstract run(): void;
}

abstract class B extends A {
  abstract run(): void;
}