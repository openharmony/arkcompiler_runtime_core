/*---
desc: Check interface with protected access modifier in derived class.
assert: Member or constructor with protected modifier can be accessed only within their declaring class and classes derived from their declaring class.
---*/

class A {
  protected interface I { fee(): int }
}

class B extends A {

  inner class C implements this.I {
    public override fee(): int {
      return 1;
    }
  }
}

function main(): void {
}
