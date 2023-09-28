/*---
desc: Static field initialization with keyword 'super'.
assert: It there is an initializer in a static field declaration, then the following rules apply to the initializer. It is a compile-time error if the keyword this or keyword super occurs in the initializer.
tags: [compile-only, negative]
---*/

class A {
  public y: int = 5;


}

class B extends A {
  public static x: int = super.y;
}
