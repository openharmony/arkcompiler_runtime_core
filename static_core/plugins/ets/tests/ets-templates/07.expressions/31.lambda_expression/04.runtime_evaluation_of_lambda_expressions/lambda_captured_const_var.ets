/*---
desc: lambda captured local const variable.
---*/

type func = (y: int) => int;

function main(): void {
  const i: int = 7;

  let bar: func = (y: int): int => { return y + i; }

  assert bar(5) == 12;
}