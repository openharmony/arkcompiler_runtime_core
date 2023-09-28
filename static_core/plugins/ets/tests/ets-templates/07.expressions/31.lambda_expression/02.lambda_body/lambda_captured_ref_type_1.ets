/*---
desc: lambda captured local variable reference type.
---*/

class A {
  public x: int = 5;
}

function lambdaInit(): func {
  let a: A = new A();

  return (y: int): int => { return y + a.x; }
}

type func = (y: int) => int;

function main(): void {
  let bar: func = lambdaInit();

  assert bar(5) == 10;
}