/*---
desc: lambda return reference type.
---*/

class A {
  public x: int = 5;
}

type func = (y: int) => A;

function main(): void {
  let bar: func = (x: int): A => { let a: A = new A(); a.x += x; return a; }

  assert bar(3).x == 8;
}