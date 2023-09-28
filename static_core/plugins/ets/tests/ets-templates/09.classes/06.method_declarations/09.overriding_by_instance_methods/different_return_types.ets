/*---
desc: Different return type in mA and mC.
assert: Return types may vary among methods that override each other if the return types are reference types. The notion of return-type-substitutability supports covariant returns, that is the specialization of the return type to a subtype.
tags: [compile-only]
---*/

class A {
  public m(): B {
    return new B;
  }
}

class B {

}

final class D extends B{

}

final class C extends A {
  override m(): D {
    return new D();
  }

}
