/*---
desc: lambda captured local variable reference type. change object.
---*/

class A {
  public x: int = 5;
}

function lambdaInit(): func {
  let a: A = new A();

  let bar: func = (y: int): int => { return y += a.x; }

  a.x = 10;

  return bar;
}

type func = (y: int) => int;

function main(): void {
  let bar: func = lambdaInit();

  assert bar(5) == 15;
}