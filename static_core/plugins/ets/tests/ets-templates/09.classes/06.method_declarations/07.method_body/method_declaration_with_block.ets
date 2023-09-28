/*---
desc: Method declaraion with block of code.
assert: A method body is either a block of code that implements the method or simply a semicolon, indicating the lack of an implementation.
tags: [compile-only]
---*/

class A {
  run(): void {
    let result: int = 5 + 5;
  }
}