/*---
desc: Check interface with private access modifier in class.
assert: Member or constructor with private modifier can be accessed only eithin their declaring class.
---*/

class A {
  private interface I { fee(): int }

  private inner class B implements I {
    public override fee(): int {
      return 1;
    }
  }

  public foo(): int {
    return new B().fee();
  }
}

function main(): void {
  let instance: A = new A();

  assert instance.foo() == 1;
}
