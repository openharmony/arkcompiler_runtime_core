/*---
desc: lambda captured local variable reference type.
---*/

class A {
  public x: int = 5;
}

type func = (y: int) => int;

function main(): void {
  let a: A = new A();

  let bar: func = (y: int): int => { return y + a.x; }

  assert bar(5) == 10;
}