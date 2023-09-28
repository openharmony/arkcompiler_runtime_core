/*---
desc: Check interface with protected access modifier in class.
assert: Member or constructor with protected modifier can be accessed only within their declaring class and classes derived from their declaring class.
---*/

class A {
  protected interface I { fee(): int }

  inner class B implements this.I {
    public override fee(): int {
      return 1;
    }
  }
}

function main(): void {
}
