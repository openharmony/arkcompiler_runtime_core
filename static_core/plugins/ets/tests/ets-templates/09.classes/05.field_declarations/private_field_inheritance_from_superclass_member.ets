/*---
desc: Inheritamce private field from of a superclass, if both classes are members of same class.
assert: Nevertheless, a private field is never inherited by a superclass.
tags: [negative, compile-only]
---*/

class A {
  inner class C {
    private x: int;
  }

  inner class B extends C {
    public getX(): int {
      return this.x;
    }
  }
}
