/*---
desc: Inheritance open method from superinterface with declared static method with same signature in interface.
assert: If an interface I declares a private or static method m, and the signature of m is a subsignature of a public instance method m' in a superinterface of I, and m' would otherwise be accessible to code in I, then a compile-time error occurs.
tags: [negative, compile-only]
---*/

interface A {
  foo(): int {
    return 1;
  }
}

interface B extends A {
  statics foo(): int {
    return 2;
  }
}
