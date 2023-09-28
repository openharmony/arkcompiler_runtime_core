/*---
desc: Check interface with public access modifier in class.
assert: Member or constructor with public modifier can be accessed everywhere.
---*/

public class A {
  public interface I { fee(): int }

  public inner class B implements I {
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
