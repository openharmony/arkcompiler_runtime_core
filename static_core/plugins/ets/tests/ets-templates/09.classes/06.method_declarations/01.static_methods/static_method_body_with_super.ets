/*---
desc: Static method with keyword 'super'.
assert: It is a compile-time error to refer to the current object using the keyword this or the keyword super.
tags: [compile-only, negative]
---*/

class A {
  public foo(): void {

  }
}

class B extends A {
  public static foo(): void {
    super.foo();
  }
}
