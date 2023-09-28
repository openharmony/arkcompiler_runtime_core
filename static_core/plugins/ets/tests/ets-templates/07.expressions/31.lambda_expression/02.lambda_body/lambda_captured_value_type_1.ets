/*---
desc: lambda captured local variable and invoke in other function.
---*/

type func = (y: int) => int;

function lambdaInit(): func {
  let i: int = 7;

  return (y: int): int => { return y + i; }
}

function main(): void {
  let bar: func = lambdaInit();
  assert bar(5) == 12;
}