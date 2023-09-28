/*---
desc: Method declaraion with return type.
assert: If a method is declared to have a return type, then a compile-time error occurs if the body of the method can complete normally.
tags: [compile-only, negative]
---*/

class A {
  run(): int {
    let result: int = 5 + 5;
  }
}