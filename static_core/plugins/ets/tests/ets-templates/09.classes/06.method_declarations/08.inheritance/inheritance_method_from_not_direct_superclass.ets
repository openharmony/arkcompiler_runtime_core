/*---
desc: Inheritance method of superclass that not a member of direct superclass.
assert: A class C inherits from its direct superclass all concrete methods m (both static and instance) of the superclass for which all of the following are true. m is a member of the direct superclass of C. m is public, protected, or declared with package access in the same package as C. No method declared in C has a signature that is a subsignature of the signature of m.
---*/

class A {
  public foo(): int {
    return 1;
  }
}

class B extends A {

}

class C extends B {
  fee(): int {
    return this.foo();
  }
}

function main(): void {
  let instance: C = new C();

  assert instance.fee() == 1;
}
