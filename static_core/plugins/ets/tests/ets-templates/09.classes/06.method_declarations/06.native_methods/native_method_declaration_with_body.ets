/*---
desc: Native method declaraion.
assert: It is a compile-time error is a native method has a body. Its body should be given as a semicolon only, indicating that the implementation is omitted, instead of a block.
tags: [compile-only, negative]
---*/

abstract class A {
  native run(): void { 

  }
}