/*---
desc: Static field initialization with keyword 'this'.
assert: It there is an initializer in a static field declaration, then the following rules apply to the initializer. It is a compile-time error if the keyword this or keyword super occurs in the initializer.
tags: [compile-only, negative]
---*/

class A {
  public y: int = 5;

  public static x: int = this.y;
}
