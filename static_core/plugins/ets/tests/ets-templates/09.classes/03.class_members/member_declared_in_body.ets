/*---
desc: class members
assert: Members declared in the body of the class.
---*/


final class B {
  public x: int = 4;

  public fee(): int {
    return 5;
  }
}

function main(): void {
    let y: B = new B();
    
    assert y.fee() == 5;
    assert y.x == 4;
}
