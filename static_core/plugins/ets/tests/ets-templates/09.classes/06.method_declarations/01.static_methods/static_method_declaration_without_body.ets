/*---
desc: Static method declaraion. Must have a body. Must have code block in the body.
assert: A method that is declared static is called a class method.
tags: [compile-only, negative]
---*/

class A {
  static run(): void;
}
