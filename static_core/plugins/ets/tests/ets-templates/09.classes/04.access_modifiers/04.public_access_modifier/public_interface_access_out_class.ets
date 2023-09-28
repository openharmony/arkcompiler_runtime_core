/*---
desc: Check interface with public access modifier out class.
assert: Member or constructor with public modifier can be accessed everywhere.
---*/

public class A {
  public interface I { fee(): int }
}

public class B implements A.I {
  public override fee(): int {
    return 1;
  }
}

function main(): void {
}
