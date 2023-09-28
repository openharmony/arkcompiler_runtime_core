/*---
desc: Override public default method from superinterface with wong signature.
assert: An instance method mI declared in or inherited by interface I, overrides from I another instance method mJ declared in interface J, iff all of the following are true - The signature of mI is a subsignature of the signature of mJ.
tags: [negative, compile-only]
---*/

interface A {
  foo(): int {
    return 1;
  }
}

interface B extends A {
  override foo(): boolean {
    return true;
  }
}
