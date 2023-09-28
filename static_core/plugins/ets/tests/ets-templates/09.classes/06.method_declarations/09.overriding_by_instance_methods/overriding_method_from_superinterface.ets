/*---
desc: Overriding method of superinterface.
assert: An instance method mC declared in or inherited by class C, overrides from C another method mI declared in interface I, iff all of the following are true. I is a superinterface of C. mI is not static. C does not inherit mI. The signature of mC is a subsignature (link to signatures) of the signature of mI. mI is public.
---*/

interface I {
  m(): int;
}

class C implements I {
  override m(): int {
    return 5;
  }
}

function main(): void {
  let instance: C = new C();

  assert instance.m() == 5;
}