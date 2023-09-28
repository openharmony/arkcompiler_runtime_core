/*---
desc: Check that readonly static field initialized first.
assert: Note that static fields that are readonly variables are initialized before other static fields.
---*/

class A {
  public static readonly y: int = 5;

  public static x: int = A.y;
}

function main(): void {
    assert A.x == 5;
}
