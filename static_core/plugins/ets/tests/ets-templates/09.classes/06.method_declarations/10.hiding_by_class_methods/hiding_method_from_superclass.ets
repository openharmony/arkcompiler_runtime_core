/*---
desc: Hiding method of superclass.
assert: If a class C declares or inherits a static method m, then m is said to hide any method m', where the signature of m is a subsignature  of the signature of m', in the superclasses and superinterfaces of C that would otherwise be accessible  to code in C.
---*/

class A {
 public static m(): int {
    return 1;
  }
}

class C extends A {
  public static m(): int {
    return 5;
  }
}

function main(): void {
  assert C.m() == 5;
}
