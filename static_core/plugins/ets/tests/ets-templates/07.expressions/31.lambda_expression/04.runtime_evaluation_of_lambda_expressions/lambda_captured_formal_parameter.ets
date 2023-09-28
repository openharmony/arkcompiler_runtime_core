/*---
desc: lambda captured formal parameter.
---*/

type func = (y: int) => int;

function lambdaInit(x: int): void {
  let bar: func = (y: int): int => { return y + x; }

  assert bar(5) == 11;
}

function main(): void {
  lambdaInit(6);
}