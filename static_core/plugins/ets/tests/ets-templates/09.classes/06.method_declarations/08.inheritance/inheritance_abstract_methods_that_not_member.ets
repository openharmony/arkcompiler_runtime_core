/*---
desc: Inheritance abstract method of superclass that not a member of direct superclass.
assert: A class C inherits from its direct superclass and direct superinterfaces all abstract and default methods m for which all of the following are true. m is a member of the direct superclass or a direct superinterface, D, of C. m is public, protected, or declared with package access in the same package as C. No method declared in C has a signature that is a subsignature of the signature of m. No concrete method inherited by C from its direct superclass has a signature that is a subsignature of the signature of m. There exists no method m' that is a member of the direct superclass or a direct superinterface, D', of C (m distinct from m', D distinct from D'), such that m' overrides from D' the declaration of the method m.
tags: [compile-only, negative]
---*/

abstract class A {
  abstract a(): int;
}

abstract class B {

}

interface D {
  d(): int {
    return 1;
  }
}

class C extends B implements D {
  fee(): int {
    return a() + d();
  }
}
