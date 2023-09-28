/*---
desc: Declaring method with same name.
assert: It is a compile-time error if a type declaration T has a member method m1 and there exists a method m2 declared in T or a supertype of T such that all of the following are true. m1 and m2 have the same name. m2 is accessible from T . The signature of m1 is not a subsignature of the signature of m2. The signature of m1 or some method m1 overrides (directly or indirectly) has the same erasure as the signature of m2 or some method m2 overrides (directly or indirectly).
tags: [compile-only, negative]
---*/

class C {
  public m(): int {
    return 5;
  }

  public m(): boolean {
    return true;
  }
}