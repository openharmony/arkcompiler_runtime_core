/*---
desc: Native method declaraion with block of code.
assert: It is a compile-time error if a method declaration is either abstract or native and has a block for its body. 
tags: [compile-only, negative]
---*/

class A {
  native run(): void {
    let result: int = 5 + 5;
  }
}