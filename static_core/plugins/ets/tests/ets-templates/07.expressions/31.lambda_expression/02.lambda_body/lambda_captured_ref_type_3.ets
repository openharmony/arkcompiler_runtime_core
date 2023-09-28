/*---
desc: lambda captured local variable reference type. change object in lambda body.
---*/

class A {
  public x: int = 5;
}

type func = (y: int) => void;

function main(): void {
  let a: A = new A();

  let bar: func = (y: int): void => { a.x += y; }
  bar(5);
  assert a.x == 10;
}