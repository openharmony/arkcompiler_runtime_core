/*---
desc: lambda captured local variable.
---*/

type func = (y: int) => int;

function main(): void {
  let i: int = 7;

  let bar: func = (y: int): int => { return y + i; }

  assert bar(5) == 12;
}