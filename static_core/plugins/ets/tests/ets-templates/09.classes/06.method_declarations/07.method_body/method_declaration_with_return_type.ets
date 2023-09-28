/*---
desc: Method declaraion with return type.
assert: If a method is declared to have a return type, then a compile-time error occurs if the body of the method can complete normally.
tags: [compile-only]
---*/

class A {
  run(): int {
    return 5;
  }
}