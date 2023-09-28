/*---
desc: Method declaraion with simply semicolon.
assert: A method body is either a block of code that implements the method or simply a semicolon, indicating the lack of an implementation. It is a compile-time error if a method declaration is neither abstract nor native and has a semicolon for its body.
tags: [compile-only, negative]
---*/

class A {
  run(): void;
}