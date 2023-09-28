/*---
desc: Static method with keyword 'this'.
assert: It is a compile-time error to refer to the current object using the keyword this or the keyword super.
tags: [compile-only, negative]
---*/

class A {
  public x:int;

  public static foo(): void {
    this.x = 1;
  }
}
