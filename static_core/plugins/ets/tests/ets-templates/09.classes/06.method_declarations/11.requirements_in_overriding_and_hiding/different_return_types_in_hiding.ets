/*---
desc: Different return type in mA and mC declared by hiding .
assert: If a method declaration d1 with return type R1 overrides or hides the declaration of another method d2 with return type R2, then d1 must be return-type-substitutable for d2, or a compile-time error occurs.
tags: [compile-only, negative]
---*/

class A {
  public m(): D {
    return new D;
  }
}

class B {

}

class D {

}

class C extends A {
  public m(): B {
    return new D();
  }

}
