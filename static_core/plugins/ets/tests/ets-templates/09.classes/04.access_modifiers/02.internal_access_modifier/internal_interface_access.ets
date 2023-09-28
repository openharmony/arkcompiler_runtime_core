/*---
desc: Check interface with internal access modifier.
assert: Member or constructor with private modifier can be accessed only eithin their declaring class.
---*/

class A {
  internal interface I { fee(): int }
}

class B implements A.I {
  public override fee(): int {
    return 1;
  }
}

function main(): void {
}
