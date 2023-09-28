/*---
desc: Check interface with private access modifier out class.
assert: Member or constructor with private modifier can be accessed only within their declaring class.
tags: [negative, compile-only]
---*/

class A {
  private interface I { fee(): int }
}

class B implements A.I {
  public override fee(): int {
    return 1;
  }
}

function main(): void {
}
