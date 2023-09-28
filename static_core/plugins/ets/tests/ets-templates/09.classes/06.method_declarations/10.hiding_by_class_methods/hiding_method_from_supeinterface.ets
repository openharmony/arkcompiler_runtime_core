/*---
desc: Hiding method of superinterface.
assert: If a class C declares or inherits a static method m, then m is said to hide any method m', where the signature of m is a subsignature  of the signature of m', in the superclasses and superinterfaces of C that would otherwise be accessible  to code in C.
---*/

interface I {
 static m(): int {
    return 1;
  }
}

class C implements I {
  public static m(): int {
    return 5;
  }
}

function main(): void {
  assert C.m() == 5;
}