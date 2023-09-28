/*---
desc: Non-static method declaraion.
assert: A method that is not declared static called an instance method, and sometimes called a non-static method.
tags: [compile-only]
---*/

class A {
  run(): void {
    
  }

  static foo(): void {

  }
}
